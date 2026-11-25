// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "occ_c_bridge.h"
#include "handle_table.h"

// OpenCASCADE（只允许出现在中间层内部）
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Quaternion.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <exception>
#include <string>

namespace {

#define OCB_CATCH_RETURN(ret)                                              \
    catch (const Standard_Failure& f) {                                    \
        setError(OCB_ERR_OCC_EXCEPTION, f.GetMessageString());             \
        return ret;                                                        \
    } catch (const std::exception& e) {                                      \
        setError(OCB_ERR_OCC_EXCEPTION, e.what());                         \
        return ret;                                                        \
    } catch (...) {                                                        \
        setError(OCB_ERR_OCC_EXCEPTION, "unknown exception");              \
        return ret;                                                        \
    }

} // namespace

extern "C" {

const char* OCB_CALL ocb_version(void) {
    return "1.5.0";
}

OcbStatus OCB_CALL ocb_last_status(void) {
    return bridgeErrStatus();
}

const char* OCB_CALL ocb_last_error(void) {
    return bridgeErrText().c_str();
}

uint64_t OCB_CALL ocb_live_count(void) {
    return HandleTable::instance().liveCount();
}

OcbPnt OCB_CALL ocb_point_xyz(double x, double y, double z) {
    OcbPnt p;
    p.x = x;
    p.y = y;
    p.z = z;
    return p;
}

OcbDir OCB_CALL ocb_dir_xyz(double dx, double dy, double dz) {
    OcbDir d;
    d.dx = dx;
    d.dy = dy;
    d.dz = dz;
    return d;
}

double OCB_CALL ocb_point_distance(OcbPnt a, OcbPnt b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

OcbTrsf OCB_CALL ocb_trsf_identity(void) {
    OcbTrsf t;
    for (int i = 0; i < 12; ++i) {
        t.v[i] = 0.0;
    }
    t.v[0] = 1.0;
    t.v[5] = 1.0;
    t.v[10] = 1.0;
    return t;
}

OcbTrsf OCB_CALL ocb_trsf_translation(OcbVec v) {
    OcbTrsf t = ocb_trsf_identity();
    t.v[3] = v.x;
    t.v[7] = v.y;
    t.v[11] = v.z;
    return t;
}

OcbPnt OCB_CALL ocb_trsf_apply(OcbTrsf t, OcbPnt p) {
    OcbPnt r;
    r.x = t.v[0] * p.x + t.v[1] * p.y + t.v[2] * p.z + t.v[3];
    r.y = t.v[4] * p.x + t.v[5] * p.y + t.v[6] * p.z + t.v[7];
    r.z = t.v[8] * p.x + t.v[9] * p.y + t.v[10] * p.z + t.v[11];
    return r;
}

OcbStatus OCB_CALL ocb_shape_make_box(
    OcbPnt corner, double dx, double dy, double dz, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    if (dx <= 0.0 || dy <= 0.0 || dz <= 0.0) {
        setError(OCB_ERR_INVALID_ARG, "box dimensions must be positive");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Pnt p(corner.x, corner.y, corner.z);
        TopoDS_Shape shape = BRepPrimAPI_MakeBox(p, dx, dy, dz).Shape();
        *out = HandleTable::instance().add(std::move(shape));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_bounding_box(OcbShape shape, OcbBox* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Bnd_Box box;
        BRepBndLib::Add(*s, box);
        if (box.IsVoid()) {
            out->valid = false;
            return OCB_OK;
        }
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        out->xmin = xmin;
        out->ymin = ymin;
        out->zmin = zmin;
        out->xmax = xmax;
        out->ymax = ymax;
        out->zmax = zmax;
        out->valid = true;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_bounding_box_ex(OcbShape shape, bool useTriangulation, OcbBox* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Bnd_Box box;
        BRepBndLib::Add(*s, box, useTriangulation ? Standard_True : Standard_False);
        if (box.IsVoid()) {
            out->valid = false;
            return OCB_OK;
        }
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        out->xmin = xmin;
        out->ymin = ymin;
        out->zmin = zmin;
        out->xmax = xmax;
        out->ymax = ymax;
        out->zmax = zmax;
        out->valid = true;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ---------------- 盒运算（Bnd_Box 最小替代） ---------------- */

namespace {

Bnd_Box toBnd(const OcbBox& b) {
    Bnd_Box box;
    if (b.valid) box.Update(b.xmin, b.ymin, b.zmin, b.xmax, b.ymax, b.zmax);
    return box;
}

void fromBnd(const Bnd_Box& box, OcbBox* out) {
    if (!out) return;
    if (box.IsVoid()) { out->valid = false; return; }
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    out->xmin = xmin; out->ymin = ymin; out->zmin = zmin;
    out->xmax = xmax; out->ymax = ymax; out->zmax = zmax;
    out->valid = true;
}

OcbTrsf toOcbTrsf(const gp_Trsf& t) {
    OcbTrsf r{};
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col) {
            r.v[(row - 1) * 4 + (col - 1)] = t.Value(row, col);
        }
    }
    return r;
}

}  // namespace

void OCB_CALL ocb_box_from_shape(OcbShape shape, bool useTriangulation, OcbBox* out) {
    if (!out) return;
    out->valid = false;
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) return;
    try {
        Bnd_Box box;
        BRepBndLib::Add(*s, box, useTriangulation ? Standard_True : Standard_False);
        fromBnd(box, out);
    }
    catch (...) {
        out->valid = false;
    }
}

bool OCB_CALL ocb_box_is_out(const OcbBox* a, const OcbBox* b) {
    if (!a || !b || !a->valid || !b->valid) return true;
    Bnd_Box A = toBnd(*a), B = toBnd(*b);
    return A.IsOut(B) ? true : false;
}

bool OCB_CALL ocb_box_is_out_gap(const OcbBox* a, const OcbBox* b, double gap) {
    if (!a || !b || !a->valid || !b->valid) return true;
    Bnd_Box A = toBnd(*a), B = toBnd(*b);
    if (gap > 0.0) { A.Enlarge(gap); }
    return A.IsOut(B) ? true : false;
}

void OCB_CALL ocb_box_add(OcbBox* dst, const OcbBox* src) {
    if (!dst || !src || !src->valid) return;
    Bnd_Box A = toBnd(*dst), B = toBnd(*src);
    A.Add(B);
    fromBnd(A, dst);
}

void OCB_CALL ocb_box_transformed(const OcbBox* box, OcbTrsf trsf, OcbBox* out) {
    if (!out || !box || !box->valid) { if (out) out->valid = false; return; }
    Bnd_Box A = toBnd(*box);
    gp_Trsf t;
    t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
    Bnd_Box B = A.Transformed(t);
    fromBnd(B, out);
}

OcbStatus OCB_CALL ocb_shape_location_cleared(OcbShape shape, OcbShape* out) {    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Shape cleared = *s;
        cleared.Location(TopLoc_Location());
        *out = HandleTable::instance().add(std::move(cleared));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_get_location(OcbShape shape, OcbTrsf* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = toOcbTrsf(s->Location().Transformation());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shape_volume(OcbShape shape, double* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = HandleTable::instance().get<TopoDS_Shape>(shape);
    if (!s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        GProp_GProps props;
        BRepGProp::VolumeProperties(*s, props);
        *out = props.Mass();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_shape_release(OcbShape shape) {
    HandleTable::instance().release(shape);
}

} // extern "C"

namespace {

gp_Trsf toTrsf(const OcbTrsf& t) {
    gp_Trsf trsf;
    trsf.SetValues(t.v[0], t.v[1], t.v[2], t.v[3],
                   t.v[4], t.v[5], t.v[6], t.v[7],
                   t.v[8], t.v[9], t.v[10], t.v[11]);
    return trsf;
}

} // namespace

extern "C" {

OcbStatus OCB_CALL ocb_quaternion_from_euler_zyx(double rz, double ry, double rx, OcbQuat* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        gp_Quaternion q;
        q.SetEulerAngles(gp_Intrinsic_ZYX, rz, ry, rx);
        *out = OcbQuat{q.X(), q.Y(), q.Z(), q.W()};
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_quaternion_dot(OcbQuat a, OcbQuat b, double* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    const gp_Quaternion qa(a.x, a.y, a.z, a.w);
    const gp_Quaternion qb(b.x, b.y, b.z, b.w);
    *out = qa.Dot(qb);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_trsf_from_quat_translation(
    OcbQuat q, double tx, double ty, double tz, OcbTrsf* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Quaternion quat(q.x, q.y, q.z, q.w);
        gp_Trsf trsf;
        trsf.SetRotation(quat);
        trsf.SetTranslationPart(gp_Vec(tx, ty, tz));
        for (int row = 1; row <= 3; ++row) {
            for (int col = 1; col <= 4; ++col) {
                out->v[(row - 1) * 4 + (col - 1)] = trsf.Value(row, col);
            }
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_trsf_get_quat(OcbTrsf t, OcbQuat* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Quaternion q = toTrsf(t).GetRotation();
        *out = OcbQuat{q.X(), q.Y(), q.Z(), q.W()};
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_trsf_get_euler_zyx(OcbTrsf t, double* rz, double* ry, double* rx) {
    if (!rz || !ry || !rx) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        const gp_Quaternion q = toTrsf(t).GetRotation();
        q.GetEulerAngles(gp_Intrinsic_ZYX, *rz, *ry, *rx);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

} // extern "C"
