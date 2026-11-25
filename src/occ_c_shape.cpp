// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "occ_c_bridge.h"
#include "handle_table.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRep_Tool.hxx>
#include <gp_Lin.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <IMeshTools_Parameters.hxx>
#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopLoc_Location.hxx>
#include <TopAbs.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <string>

namespace {

#define OCB_CATCH_RETURN(ret)                                              \
    catch (const Standard_Failure& f) {                                    \
        setError(OCB_ERR_OCC_EXCEPTION, f.GetMessageString());             \
        return ret;                                                        \
    } catch (const std::exception& e) {                                    \
        setError(OCB_ERR_OCC_EXCEPTION, e.what());                         \
        return ret;                                                        \
    } catch (...) {                                                        \
        setError(OCB_ERR_OCC_EXCEPTION, "unknown exception");              \
        return ret;                                                        \
    }

HandleTable& table() {
    return HandleTable::instance();
}

gp_Pnt toPnt(OcbPnt p) { return gp_Pnt(p.x, p.y, p.z); }
OcbPnt toOcb(const gp_Pnt& p) { return OcbPnt{p.X(), p.Y(), p.Z()}; }
OcbVec toOcb(const gp_Vec& v) { return OcbVec{v.X(), v.Y(), v.Z()}; }
OcbDir toOcb(const gp_Dir& d) { return OcbDir{d.X(), d.Y(), d.Z()}; }

gp_Trsf toTrsf(OcbTrsf t) {
    gp_Trsf trsf;
    trsf.SetValues(t.v[0], t.v[1], t.v[2], t.v[3],
                   t.v[4], t.v[5], t.v[6], t.v[7],
                   t.v[8], t.v[9], t.v[10], t.v[11]);
    return trsf;
}

OcbTrsf fromTrsf(const gp_Trsf& trsf) {
    OcbTrsf r;
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col) {
            r.v[(row - 1) * 4 + (col - 1)] = trsf.Value(row, col);
        }
    }
    return r;
}

} // namespace

extern "C" {

OcbVec OCB_CALL ocb_vec_xyz(double x, double y, double z) {
    return OcbVec{x, y, z};
}

double OCB_CALL ocb_vec_modulus(OcbVec v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

OcbPnt OCB_CALL ocb_point_transformed(OcbPnt p, OcbTrsf t) {
    return toOcb(toPnt(p).Transformed(toTrsf(t)));
}

OcbTrsf OCB_CALL ocb_trsf_rotation(OcbPnt origin, OcbDir axis, double angle_rad) {
    gp_Trsf trsf;
    trsf.SetRotation(gp_Ax1(toPnt(origin), gp_Dir(axis.dx, axis.dy, axis.dz)), angle_rad);
    return fromTrsf(trsf);
}

OcbTrsf OCB_CALL ocb_trsf_set_values(
    double v11, double v12, double v13, double v14,
    double v21, double v22, double v23, double v24,
    double v31, double v32, double v33, double v34) {
    gp_Trsf trsf;
    trsf.SetValues(v11, v12, v13, v14, v21, v22, v23, v24, v31, v32, v33, v34);
    return fromTrsf(trsf);
}

OcbVec OCB_CALL ocb_trsf_translation_part(OcbTrsf t) {
    const gp_Trsf trsf = toTrsf(t);
    const gp_XYZ& part = trsf.TranslationPart();
    return OcbVec{part.X(), part.Y(), part.Z()};
}

OcbStatus OCB_CALL ocb_trsf_from_native(const void* nativeTrsf, OcbTrsf* out) {
    if (!nativeTrsf || !out) {
        setError(OCB_ERR_INVALID_ARG, "nativeTrsf or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = fromTrsf(*static_cast<const gp_Trsf*>(nativeTrsf));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbTrsf OCB_CALL ocb_trsf_multiply(OcbTrsf a, OcbTrsf b) {
    return fromTrsf(toTrsf(a).Multiplied(toTrsf(b)));
}

OcbTrsf OCB_CALL ocb_trsf_inverted(OcbTrsf t) {
    return fromTrsf(toTrsf(t).Inverted());
}

OcbStatus OCB_CALL ocb_make_cylinder(OcbAx2 axis, double r, double h, OcbShape* out) {
    if (!out || r <= 0.0 || h <= 0.0) {
        setError(OCB_ERR_INVALID_ARG, "invalid cylinder args");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        gp_Ax2 ax2(toPnt(axis.origin), gp_Dir(axis.normal.dx, axis.normal.dy, axis.normal.dz),
                   gp_Dir(axis.xdir.dx, axis.xdir.dy, axis.xdir.dz));
        *out = table().add(BRepPrimAPI_MakeCylinder(ax2, r, h).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_cone(OcbAx2 axis, double r1, double r2, double h, OcbShape* out) {
    if (!out || h <= 0.0) {
        setError(OCB_ERR_INVALID_ARG, "invalid cone args");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        gp_Ax2 ax2(toPnt(axis.origin), gp_Dir(axis.normal.dx, axis.normal.dy, axis.normal.dz),
                   gp_Dir(axis.xdir.dx, axis.xdir.dy, axis.xdir.dz));
        *out = table().add(BRepPrimAPI_MakeCone(ax2, r1, r2, h).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_sphere(OcbPnt center, double r, OcbShape* out) {
    if (!out || r <= 0.0) {
        setError(OCB_ERR_INVALID_ARG, "invalid sphere args");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(BRepPrimAPI_MakeSphere(toPnt(center), r).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_fuse(OcbShape a, OcbShape b, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE, "fuse args");
        return sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepAlgoAPI_Fuse(*sa, *sb).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_cut(OcbShape a, OcbShape b, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE, "cut args");
        return sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepAlgoAPI_Cut(*sa, *sb).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_common(OcbShape a, OcbShape b, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE, "common args");
        return sa && sb ? OCB_ERR_INVALID_ARG : OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepAlgoAPI_Common(*sa, *sb).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_fuse_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAlgoAPI_Fuse algo(*sa, *sb);
        algo.SetFuzzyValue(fuzzy);
        algo.SetRunParallel(parallel ? Standard_True : Standard_False);
        algo.Build();
        if (!algo.IsDone() || algo.Shape().IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "fuse failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(algo.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_cut_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAlgoAPI_Cut algo(*sa, *sb);
        algo.SetFuzzyValue(fuzzy);
        algo.SetRunParallel(parallel ? Standard_True : Standard_False);
        algo.Build();
        if (!algo.IsDone() || algo.Shape().IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "cut failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(algo.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* Common with explicit OBB switch. Thin/co-planar shallow embedding (PCB layers,
   mated parts) needs OBB; without it BOP silently returns empty for co-planar input.
   The old signature is kept unchanged. */
OcbStatus OCB_CALL ocb_bool_common_ex2(OcbShape a, OcbShape b, double fuzzy, bool parallel,
                                      bool useObb, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAlgoAPI_Common algo(*sa, *sb);
        algo.SetFuzzyValue(fuzzy);
        algo.SetRunParallel(parallel ? Standard_True : Standard_False);
        algo.SetUseOBB(useObb ? Standard_True : Standard_False);
        algo.Build();
        if (!algo.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "common failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(algo.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_bool_common_ex(OcbShape a, OcbShape b, double fuzzy, bool parallel, OcbShape* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAlgoAPI_Common algo(*sa, *sb);
        algo.SetFuzzyValue(fuzzy);
        algo.SetRunParallel(parallel ? Standard_True : Standard_False);
        algo.Build();
        if (!algo.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "common failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(algo.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_is_null(OcbShape shape, bool* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = s->IsNull();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shape_is_equal(OcbShape a, OcbShape b, bool* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handles");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = sa->IsEqual(*sb);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shape_type(OcbShape shape, int* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = static_cast<int>(s->ShapeType());
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shape_distance(OcbShape a, OcbShape b, double* out) {
    TopoDS_Shape* sa = table().get<TopoDS_Shape>(a);
    TopoDS_Shape* sb = table().get<TopoDS_Shape>(b);
    if (!out || !sa || !sb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepExtrema_DistShapeShape dist(*sa, *sb);
        if (!dist.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "distance failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = dist.Value();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_classifier_new(OcbShape shape, OcbCls* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<BRepClass3d_SolidClassifier>(
            new BRepClass3d_SolidClassifier(*s)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* state 取值: 0=TopAbs_IN, 1=TopAbs_OUT, 2=TopAbs_ON, 3=TopAbs_UNKNOWN */
OcbStatus OCB_CALL ocb_classifier_perform(OcbCls cls, OcbPnt pnt, double tol, int* state) {
    std::shared_ptr<BRepClass3d_SolidClassifier>* c =
        table().get<std::shared_ptr<BRepClass3d_SolidClassifier>>(cls);
    if (!state || !c || !c->get()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid classifier handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*c)->Perform(gp_Pnt(pnt.x, pnt.y, pnt.z), tol);
        *state = static_cast<int>((*c)->State());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_classifier_release(OcbCls cls) {
    table().release(cls);
}

OcbStatus OCB_CALL ocb_fillet_new(OcbShape base, OcbFillet* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(base);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<BRepFilletAPI_MakeFillet>(
            new BRepFilletAPI_MakeFillet(*s)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_fillet_add(OcbFillet fillet, OcbShape edge, double radius) {
    std::shared_ptr<BRepFilletAPI_MakeFillet>* mk =
        table().get<std::shared_ptr<BRepFilletAPI_MakeFillet>>(fillet);
    TopoDS_Shape* e = table().get<TopoDS_Shape>(edge);
    if (!mk || !mk->get() || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid fillet/edge handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (e->ShapeType() != TopAbs_EDGE) {
            setError(OCB_ERR_INVALID_HANDLE, "shape is not an edge");
            return OCB_ERR_INVALID_HANDLE;
        }
        TopoDS_Edge te = TopoDS::Edge(*e);
        (*mk)->Add(radius, te);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_fillet_build(OcbFillet fillet, OcbShape* out) {
    std::shared_ptr<BRepFilletAPI_MakeFillet>* mk =
        table().get<std::shared_ptr<BRepFilletAPI_MakeFillet>>(fillet);
    if (!out || !mk || !mk->get()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid fillet handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*mk)->Build();
        if (!(*mk)->IsDone() || (*mk)->Shape().IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "fillet failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add((*mk)->Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_fillet_release(OcbFillet fillet) {
    table().release(fillet);
}

OcbStatus OCB_CALL ocb_chamfer_new(OcbShape base, OcbChamfer* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(base);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<BRepFilletAPI_MakeChamfer>(
            new BRepFilletAPI_MakeChamfer(*s)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_chamfer_add(OcbChamfer chamfer, OcbShape edge, double dist) {
    std::shared_ptr<BRepFilletAPI_MakeChamfer>* mk =
        table().get<std::shared_ptr<BRepFilletAPI_MakeChamfer>>(chamfer);
    TopoDS_Shape* e = table().get<TopoDS_Shape>(edge);
    if (!mk || !mk->get() || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid chamfer/edge handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (e->ShapeType() != TopAbs_EDGE) {
            setError(OCB_ERR_INVALID_HANDLE, "shape is not an edge");
            return OCB_ERR_INVALID_HANDLE;
        }
        TopoDS_Edge te = TopoDS::Edge(*e);
        (*mk)->Add(dist, te);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_chamfer_build(OcbChamfer chamfer, OcbShape* out) {
    std::shared_ptr<BRepFilletAPI_MakeChamfer>* mk =
        table().get<std::shared_ptr<BRepFilletAPI_MakeChamfer>>(chamfer);
    if (!out || !mk || !mk->get()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid chamfer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*mk)->Build();
        if (!(*mk)->IsDone() || (*mk)->Shape().IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "chamfer failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add((*mk)->Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_chamfer_release(OcbChamfer chamfer) {
    table().release(chamfer);
}

OcbStatus OCB_CALL ocb_make_pipe(OcbShape spine_wire, OcbShape profile_face, OcbShape* out) {
    TopoDS_Shape* w = table().get<TopoDS_Shape>(spine_wire);
    TopoDS_Shape* f = table().get<TopoDS_Shape>(profile_face);
    if (!out || !w || !f) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid spine/profile handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (w->ShapeType() != TopAbs_WIRE) {
            setError(OCB_ERR_INVALID_HANDLE, "shape is not a wire");
            return OCB_ERR_INVALID_HANDLE;
        }
        TopoDS_Wire wire = TopoDS::Wire(*w);
        BRepOffsetAPI_MakePipe maker(wire, *f);
        if (!maker.IsDone() || maker.Shape().IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "pipe failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(maker.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_moved(OcbShape shape, OcbTrsf t, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Shape moved = s->Moved(TopLoc_Location(toTrsf(t)));
        *out = table().add(std::move(moved));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_copy(OcbShape shape, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepBuilderAPI_Copy copier(*s);
        if (!copier.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "copy failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(copier.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_make_compound(OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        TopoDS_Compound compound;
        BRep_Builder builder;
        builder.MakeCompound(compound);
        *out = table().add(TopoDS_Shape(compound));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_add(OcbShape compound, OcbShape part) {
    TopoDS_Shape* c = table().get<TopoDS_Shape>(compound);
    TopoDS_Shape* s = table().get<TopoDS_Shape>(part);
    if (!c || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid compound/part handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRep_Builder builder;
        builder.Add(*c, *s);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_count_edges(OcbShape shape, int* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    TopTools_IndexedMapOfShape uniqueEdges;
    TopExp::MapShapes(*s, TopAbs_EDGE, uniqueEdges);
    *out = uniqueEdges.Extent();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_topo_count_solids(OcbShape shape, int* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        int n = 0;
        for (TopExp_Explorer ex(*s, TopAbs_SOLID); ex.More(); ex.Next()) ++n;
        *out = n;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_count_faces(OcbShape shape, int* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        int n = 0;
        for (TopExp_Explorer ex(*s, TopAbs_FACE); ex.More(); ex.Next()) ++n;
        *out = n;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_compound_first(OcbShape shape, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Iterator it(*s);
        if (!it.More()) {
            setError(OCB_ERR_INVALID_ARG, "shape has no child");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(it.Value());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_get_solid(OcbShape shape, int index, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (index < 0) {
        setError(OCB_ERR_INVALID_ARG, "solid index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        int i = 0;
        for (TopExp_Explorer ex(*s, TopAbs_SOLID); ex.More(); ex.Next(), ++i) {
            if (i == index) {
                // 以 TopoDS_Shape 类型登记（句柄表按类型指纹校验，登记成 TopoDS_Solid 会让
                // 后续按 TopoDS_Shape 取的 API 全部失配）
                *out = table().add(ex.Current());
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "solid index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_get_child(OcbShape shape, int index, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Iterator it(*s);
        int i = 0;
        for (; it.More(); it.Next(), ++i) {
            if (i == index) {
                *out = table().add(it.Value());
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "child index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_edge_from_shape(OcbShape shape, OcbEdge* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Edge edge = TopoDS::Edge(*s);
        if (edge.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "shape is not an edge");
            return OCB_ERR_OCC_EXCEPTION;
        }
        // 必须按 TopoDS_Shape 存：读取方（edge_first_param/edge_curve 等）按 Shape 取，
        // 存 Edge 会因表内类型不匹配导致句柄失效（2026-11-18 单测发现）
        TopoDS_Shape edgeShape = edge;
        *out = table().add(edgeShape);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_vertex_from_shape(OcbShape shape, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Vertex vertex = TopoDS::Vertex(*s);
        if (vertex.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "shape is not a vertex");
            return OCB_ERR_OCC_EXCEPTION;
        }
        TopoDS_Shape vertexShape = vertex;
        *out = table().add(vertexShape);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_topo_get_vertex(OcbShape shape, int index, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopExp_Explorer exp(*s, TopAbs_VERTEX);
        int i = 0;
        for (; exp.More(); exp.Next(), ++i) {
            if (i == index) {
                *out = table().add(exp.Current());
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "vertex index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_vertex_pnt(OcbShape vertex, OcbPnt* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(vertex);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid vertex handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const TopoDS_Vertex& v = TopoDS::Vertex(*s);
        const gp_Pnt p = BRep_Tool::Pnt(v);
        *out = toOcb(p);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_edge_first_param(OcbEdge edge, double* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(edge);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAdaptor_Curve curve(TopoDS::Edge(*s));
        *out = curve.FirstParameter();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_edge_last_param(OcbEdge edge, double* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(edge);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAdaptor_Curve curve(TopoDS::Edge(*s));
        *out = curve.LastParameter();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_edge_curve(OcbEdge edge, double* first, double* last, OcbCurve* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(edge);
    if (!first || !last || !out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle or output");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        double f = 0.0;
        double l = 0.0;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(TopoDS::Edge(*s), f, l);
        if (curve.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "edge has no 3D curve");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *first = f;
        *last = l;
        *out = table().add(curve);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_curve_value(OcbCurve curve, double param, OcbPnt* out) {
    Handle(Geom_Curve)* c = table().get<Handle(Geom_Curve)>(curve);
    if (!out || !c || c->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid curve handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = toOcb((*c)->Value(param));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_curve_release(OcbCurve curve) {
    table().release(curve);
}

OcbStatus OCB_CALL ocb_curve_polyline(OcbCurve curve, double firstParam, double lastParam,
    double deflection, OcbPnt* buffer, int capacity, int* outCount) {
    if (!outCount) {
        setError(OCB_ERR_INVALID_ARG, "outCount is null");
        return OCB_ERR_INVALID_ARG;
    }
    *outCount = 0;
    Handle(Geom_Curve)* c = table().get<Handle(Geom_Curve)>(curve);
    if (!c || c->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid curve handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (!buffer || capacity <= 0) {
        setError(OCB_ERR_INVALID_ARG, "buffer is null/too small");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        // 直线/无限曲线必须给参数区间，否则弦高采样会发散（实测 ±2e100）
        GeomAdaptor_Curve adapt(*c, firstParam, lastParam);
        GCPnts_QuasiUniformDeflection sampler(adapt, deflection > 1e-9 ? deflection : 1e-3);
        if (!sampler.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "discretization failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        const Standard_Integer n = sampler.NbPoints();
        const Standard_Integer count = n < capacity ? n : capacity;
        for (Standard_Integer i = 0; i < count; ++i) {
            buffer[i] = toOcb(sampler.Value(i + 1));
        }
        *outCount = (int)count;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_ray_intersect(OcbShape shape, OcbPnt origin, OcbDir dir,
    double* outParam, OcbPnt* outPnt, int* outFaceIndex) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (!outParam || !outPnt || !outFaceIndex) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Pnt o(origin.x, origin.y, origin.z);
        const gp_Dir d(dir.dx, dir.dy, dir.dz);
        IntCurvesFace_ShapeIntersector inter;
        inter.Load(*s, 1e-6);
        inter.Perform(gp_Lin(o, d), -1e30, 1e30);
        if (!inter.IsDone() || inter.NbPnt() <= 0) {
            *outParam = 0.0;
            *outPnt = OcbPnt{ o.X(), o.Y(), o.Z() };
            *outFaceIndex = -1;
            return OCB_OK;   // 无交点：返回 OK + faceIndex=-1
        }
        // 取最小参数（最近交点）
        int best = 1;
        for (Standard_Integer i = 2; i <= inter.NbPnt(); ++i) {
            if (inter.WParameter(i) < inter.WParameter(best)) best = i;
        }
        *outParam = inter.WParameter(best);
        *outPnt = toOcb(inter.Pnt(best));
        *outFaceIndex = best - 1;   // 命中序号（0-based）；面实体索引不在本接口暴露
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_edge_pnt_pnt(OcbPnt p1, OcbPnt p2, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(BRepBuilderAPI_MakeEdge(toPnt(p1), toPnt(p2)).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_edge_from_curve(OcbCurve curve, OcbShape* out) {
    Handle(Geom_Curve)* c = table().get<Handle(Geom_Curve)>(curve);
    if (!out || !c || c->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid curve handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepBuilderAPI_MakeEdge(*c).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_arc_of_circle(OcbPnt p1, OcbPnt p2, OcbPnt p3, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        GC_MakeArcOfCircle arc(toPnt(p1), toPnt(p2), toPnt(p3));
        if (!arc.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "arc construction failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(BRepBuilderAPI_MakeEdge(arc.Value()).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_wire_from_edges(OcbEdge e1, OcbEdge e2, OcbEdge e3, OcbShape* out) {
    TopoDS_Shape* s1 = table().get<TopoDS_Shape>(e1);
    TopoDS_Shape* s2 = table().get<TopoDS_Shape>(e2);
    TopoDS_Shape* s3 = table().get<TopoDS_Shape>(e3);
    if (!out || !s1 || !s2 || !s3) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handles");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepBuilderAPI_MakeWire wire(TopoDS::Edge(*s1), TopoDS::Edge(*s2), TopoDS::Edge(*s3));
        if (!wire.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "wire construction failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(wire.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 单边 wire（金丝/圆截面等单段轮廓用；make_wire_from_edges 固定 3 边不适用） */
OcbStatus OCB_CALL ocb_make_wire_from_edge(OcbShape edge, OcbShape* out) {
    TopoDS_Shape* e = table().get<TopoDS_Shape>(edge);
    if (!out || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepBuilderAPI_MakeWire mk(TopoDS::Edge(*e));
        if (!mk.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "make wire failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(TopoDS_Shape(mk.Wire()));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* N 边 wire（等价 BRepBuilderAPI_MakeWire::Add 循环；N 任意 >= 1） */
OcbStatus OCB_CALL ocb_make_wire_from_shapes(const OcbShape* edges, int n, OcbShape* out) {
    if (!edges || !out || n <= 0) {
        setError(OCB_ERR_INVALID_ARG, "edges/out is null or n <= 0");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        BRepBuilderAPI_MakeWire w;
        for (int i = 0; i < n; ++i) {
            TopoDS_Shape* e = table().get<TopoDS_Shape>(edges[i]);
            if (!e || e->IsNull()) {
                setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle in array");
                return OCB_ERR_INVALID_HANDLE;
            }
            w.Add(TopoDS::Edge(*e));
        }
        if (!w.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "wire construction failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(w.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 边上某参数处的点与切向（等价 BRepAdaptor_Curve::D1） */
OcbStatus OCB_CALL ocb_edge_point_and_tangent(OcbShape edge, double param,
                                             OcbPnt* pnt, OcbDir* tangent) {
    TopoDS_Shape* e = table().get<TopoDS_Shape>(edge);
    if (!e || !pnt || !tangent) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid edge handle or out");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAdaptor_Curve curve(TopoDS::Edge(*e));
        gp_Pnt p;
        gp_Vec v;
        curve.D1(param, p, v);
        pnt->x = p.X(); pnt->y = p.Y(); pnt->z = p.Z();
        if (v.SquareMagnitude() < 1e-24) {
            setError(OCB_ERR_INVALID_ARG, "zero tangent at param");
            return OCB_ERR_INVALID_ARG;
        }
        v.Normalize();
        tangent->dx = v.X(); tangent->dy = v.Y(); tangent->dz = v.Z();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 圆边（等价 gp_Circ + BRepBuilderAPI_MakeEdge） */
OcbStatus OCB_CALL ocb_make_circle_edge(OcbPnt center, OcbDir normal, double radius, OcbShape* out) {
    if (!out || radius <= 0.0) {
        setError(OCB_ERR_INVALID_ARG, "invalid circle args");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Ax2 ax(gp_Pnt(center.x, center.y, center.z),
                        gp_Dir(normal.dx, normal.dy, normal.dz));
        BRepBuilderAPI_MakeEdge mk(gp_Circ(ax, radius));
        if (!mk.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "make circle edge failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(TopoDS_Shape(mk.Edge()));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_face_from_wire(OcbShape wire, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(wire);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid wire handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepBuilderAPI_MakeFace face(TopoDS::Wire(*s));
        if (!face.IsDone()) {
            setError(OCB_ERR_OCC_EXCEPTION, "face construction failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(face.Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_transform(OcbShape shape, OcbTrsf t, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepBuilderAPI_Transform(*s, toTrsf(t), Standard_False).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_make_prism(OcbShape face, OcbVec vec, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(BRepPrimAPI_MakePrism(*s, gp_Vec(vec.x, vec.y, vec.z)).Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_explorer_new_faces(OcbShape shape, OcbExplorer* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<TopExp_Explorer>(
            new TopExp_Explorer(*s, TopAbs_FACE)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_explorer_new_edges(OcbShape shape, OcbExplorer* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<TopExp_Explorer>(
            new TopExp_Explorer(*s, TopAbs_EDGE)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_explorer_new_wires(OcbShape shape, OcbExplorer* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(std::shared_ptr<TopExp_Explorer>(
            new TopExp_Explorer(*s, TopAbs_WIRE)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_explorer_more(OcbExplorer exp, bool* out) {
    TopExp_Explorer* e = table().get<std::shared_ptr<TopExp_Explorer>>(exp)->get();
    if (!out || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid explorer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = e->More();
    return OCB_OK;
}

void OCB_CALL ocb_explorer_next(OcbExplorer exp) {
    if (TopExp_Explorer* e = table().get<std::shared_ptr<TopExp_Explorer>>(exp)->get()) {
        e->Next();
    }
}

OcbStatus OCB_CALL ocb_explorer_current_face(OcbExplorer exp, OcbFace* out) {
    TopExp_Explorer* e = table().get<std::shared_ptr<TopExp_Explorer>>(exp)->get();
    if (!out || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid explorer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(TopoDS_Shape(TopoDS::Face(e->Current())));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_explorer_current_edge(OcbExplorer exp, OcbEdge* out) {
    TopExp_Explorer* e = table().get<std::shared_ptr<TopExp_Explorer>>(exp)->get();
    if (!out || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid explorer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(TopoDS_Shape(TopoDS::Edge(e->Current())));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_explorer_release(OcbExplorer exp) {
    table().release(exp);
}

OcbStatus OCB_CALL ocb_explorer_current_wire(OcbExplorer exp, OcbShape* out) {
    TopExp_Explorer* e = table().get<std::shared_ptr<TopExp_Explorer>>(exp)->get();
    if (!out || !e) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid explorer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(TopoDS_Shape(TopoDS::Wire(e->Current())));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 面的朝向：0=FORWARD 1=REVERSED 2=INTERNAL 3=EXTERNAL（TopAbs_Orientation）。
   三角化绕序跟随曲面法向、不含面朝向，调用方需要它来修正上下表面判断。 */
OcbStatus OCB_CALL ocb_face_orientation(OcbFace face, int* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = (int)TopoDS::Face(*s).Orientation();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 面的外轮廓线（用于区分"腔体轮廓"与"外边界"）。无外轮廓时返回 OCB_INVALID 句柄 + OCB_OK。 */
OcbStatus OCB_CALL ocb_face_outer_wire(OcbFace face, OcbShape* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const TopoDS_Wire w = BRepTools::OuterWire(TopoDS::Face(*s));
        *out = w.IsNull() ? OCB_INVALID : table().add(TopoDS_Shape(w));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_face_is_plane(OcbFace face, bool* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAdaptor_Surface surf(TopoDS::Face(*s));
        *out = (surf.GetType() == GeomAbs_Plane);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_face_plane_normal(OcbFace face, OcbDir* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepAdaptor_Surface surf(TopoDS::Face(*s));
        if (surf.GetType() != GeomAbs_Plane) {
            setError(OCB_ERR_INVALID_ARG, "face is not planar");
            return OCB_ERR_INVALID_ARG;
        }
        const gp_Dir d = surf.Plane().Axis().Direction();
        out->dx = d.X(); out->dy = d.Y(); out->dz = d.Z();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_mesh_generate(OcbShape shape, double deflection) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!s || deflection <= 0.0) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle or deflection");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        BRepMesh_IncrementalMesh mesh(*s, deflection, Standard_False);
        return mesh.IsDone() ? OCB_OK : OCB_ERR_OCC_EXCEPTION;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 网格生成（完整参数版）：等价 IMeshTools_Parameters + BRepMesh_IncrementalMesh。
   relative=false 时 deflection 为绝对弦高（mm）。 */
OcbStatus OCB_CALL ocb_mesh_generate_ex(OcbShape shape, double deflection, double angle,
                                       double minSize, bool relative, bool inParallel) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!s || deflection <= 0.0) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle or deflection");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        IMeshTools_Parameters params;
        params.Relative = relative ? Standard_True : Standard_False;
        params.Deflection = deflection;
        params.Angle = angle > 0.0 ? angle : 0.35;
        params.MinSize = minSize;
        params.InParallel = inParallel ? Standard_True : Standard_False;
        BRepMesh_IncrementalMesh mesher(*s, params);
        return mesher.IsDone() ? OCB_OK : OCB_ERR_OCC_EXCEPTION;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_face_triangulation(OcbFace face, OcbTri* out, OcbTrsf* loc) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(face);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid face handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopLoc_Location location;
        const Handle(Poly_Triangulation)& tri =
            BRep_Tool::Triangulation(TopoDS::Face(*s), location);
        if (tri.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "face has no triangulation");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(tri);
        if (loc) {
            *loc = fromTrsf(location.Transformation());
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_tri_nb_triangles(OcbTri tri, int* out) {
    Handle(Poly_Triangulation)* t = table().get<Handle(Poly_Triangulation)>(tri);
    if (!out || !t || t->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid triangulation handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*t)->NbTriangles();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_tri_get_triangle(OcbTri tri, int index, int* n1, int* n2, int* n3) {
    Handle(Poly_Triangulation)* t = table().get<Handle(Poly_Triangulation)>(tri);
    if (!n1 || !n2 || !n3 || !t || t->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid triangulation handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const Poly_Triangle triangle = (*t)->Triangle(index);
        triangle.Get(*n1, *n2, *n3);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_tri_node(OcbTri tri, int index, OcbPnt* pnt) {
    Handle(Poly_Triangulation)* t = table().get<Handle(Poly_Triangulation)>(tri);
    if (!pnt || !t || t->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid triangulation handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const gp_Pnt p = (*t)->Node(index);
        *pnt = toOcb(p);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 任意 EntityOwner -> 拓扑形状（供拾取回调使用；原生 StdSelect_BRepOwner 路径） */
OcbStatus OCB_CALL ocb_owner_native_shape(void* owner, OcbShape* out) {
    if (!owner || !out) {
        setError(OCB_ERR_INVALID_ARG, "owner/out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Handle(SelectMgr_EntityOwner) base = Handle(SelectMgr_EntityOwner)::DownCast(
            static_cast<SelectMgr_EntityOwner*>(owner));
        Handle(StdSelect_BRepOwner) brep = Handle(StdSelect_BRepOwner)::DownCast(base);
        if (brep.IsNull() || brep->Shape().IsNull()) {
            setError(OCB_ERR_INVALID_ARG, "owner has no BRep shape");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(brep->Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_tri_release(OcbTri tri) {
    table().release(tri);
}

OcbStatus OCB_CALL ocb_gprops_volume(OcbShape shape, OcbGProps* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const OcbGProps h = table().add(std::shared_ptr<GProp_GProps>(new GProp_GProps()));
        BRepGProp::VolumeProperties(*s, *table().get<std::shared_ptr<GProp_GProps>>(h)->get());
        *out = h;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_gprops_linear(OcbShape shape, OcbGProps* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const OcbGProps h = table().add(std::shared_ptr<GProp_GProps>(new GProp_GProps()));
        BRepGProp::LinearProperties(*s, *table().get<std::shared_ptr<GProp_GProps>>(h)->get());
        *out = h;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_gprops_surface(OcbShape shape, OcbGProps* out) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!out || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const OcbGProps h = table().add(std::shared_ptr<GProp_GProps>(new GProp_GProps()));
        BRepGProp::SurfaceProperties(*s, *table().get<std::shared_ptr<GProp_GProps>>(h)->get());
        *out = h;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_gprops_mass(OcbGProps props, double* out) {
    GProp_GProps* p = table().get<std::shared_ptr<GProp_GProps>>(props)->get();
    if (!out || !p) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid gprops handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = p->Mass();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_gprops_center_of_mass(OcbGProps props, OcbPnt* out) {
    GProp_GProps* p = table().get<std::shared_ptr<GProp_GProps>>(props)->get();
    if (!out || !p) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid gprops handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = toOcb(p->CentreOfMass());
    return OCB_OK;
}

void OCB_CALL ocb_gprops_release(OcbGProps props) {
    table().release(props);
}

} // extern "C"
