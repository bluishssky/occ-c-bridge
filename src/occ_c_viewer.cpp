// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "occ_c_bridge.h"
#include "handle_table.h"

// OpenCASCADE（只允许出现在中间层内部）
#include <AIS_Shape.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <AIS_ConnectedInteractive.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Aspect_GridDrawMode.hxx>
#include <Aspect_GridType.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Aspect_TypeOfHighlightMethod.hxx>
#include <Graphic3d_Camera.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_ViewCube.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Graphic3d_TransModeFlags.hxx>
#include <Graphic3d_TypeOfShadingModel.hxx>
#include <Graphic3d_Vertex.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_TypeOfHighlight.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_ListOfLight.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>
#include <Aspect_Window.hxx>
#include <Image_AlienPixMap.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_CLight.hxx>
#include <gp_Dir.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdSelect_ViewerSelector3d.hxx>
#include <SelectMgr_Selection.hxx>
#include <Standard_Failure.hxx>

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
static_assert(OCB_ORIENT_XPOS == V3d_Xpos, "orientation drift");
static_assert(OCB_ORIENT_YPOS == V3d_Ypos, "orientation drift");
static_assert(OCB_ORIENT_ZPOS == V3d_Zpos, "orientation drift");
static_assert(OCB_ORIENT_XNEG == V3d_Xneg, "orientation drift");
static_assert(OCB_ORIENT_YNEG == V3d_Yneg, "orientation drift");
static_assert(OCB_ORIENT_ZNEG == V3d_Zneg, "orientation drift");
static_assert(OCB_ORIENT_XPOSYNEGZPOS == V3d_XposYnegZpos, "orientation drift");
static_assert(OCB_RM_RASTERIZATION == Graphic3d_RM_RASTERIZATION, "render mode drift");
static_assert(OCB_RTM_BLEND_OIT == Graphic3d_RTM_BLEND_OIT, "transparency method drift");
static_assert(OCB_TONE_FILMIC == Graphic3d_ToneMappingMethod_Filmic, "tone mapping drift");
static_assert(OCB_AIS_SHADED == AIS_Shaded, "display mode drift");
static_assert(OCB_SEL_SCHEME_REPLACE == AIS_SelectionScheme_Replace, "selection scheme drift");
static_assert(OCB_SEL_SCHEME_XOR == AIS_SelectionScheme_XOR, "selection scheme drift");
static_assert(OCB_ZLAYER_UNKNOWN == Graphic3d_ZLayerId_UNKNOWN, "zlayer enum drift");
static_assert(OCB_ZLAYER_DEFAULT == Graphic3d_ZLayerId_Default, "zlayer enum drift");
static_assert(OCB_ZLAYER_TOP     == Graphic3d_ZLayerId_Top,     "zlayer enum drift");
static_assert(OCB_ZLAYER_TOPMOST == Graphic3d_ZLayerId_Topmost, "zlayer enum drift");
static_assert(OCB_ZLAYER_TOPOSD  == Graphic3d_ZLayerId_TopOSD,  "zlayer enum drift");
static_assert(OCB_ZLAYER_BOTOSD  == Graphic3d_ZLayerId_BotOSD,  "zlayer enum drift");

static_assert(OCB_DISPLAY_PRIORITY_ABOVE   == Graphic3d_DisplayPriority_Above,   "priority drift");
static_assert(OCB_DISPLAY_PRIORITY_TOPMOST == Graphic3d_DisplayPriority_Topmost, "priority drift");

static_assert(OCB_MATERIAL_BRASS     == Graphic3d_NameOfMaterial_Brass,     "material drift");
static_assert(OCB_MATERIAL_GOLD      == Graphic3d_NameOfMaterial_Gold,      "material drift");
static_assert(OCB_MATERIAL_STEEL     == Graphic3d_NameOfMaterial_Steel,     "material drift");
static_assert(OCB_MATERIAL_STONE     == Graphic3d_NameOfMaterial_Stone,     "material drift");
static_assert(OCB_MATERIAL_COPPER    == Graphic3d_NameOfMaterial_Copper,    "material drift");
static_assert(OCB_MATERIAL_PLASTIC   == Graphic3d_NameOfMaterial_Plastified, "material drift");
static_assert(OCB_MATERIAL_NEON_GNC  == Graphic3d_NameOfMaterial_Ionized,   "material drift");

static_assert(OCB_SHADING_COLOR    == Graphic3d_TypeOfShadingModel_Unlit,     "shading model drift");
static_assert(OCB_SHADING_FLAT     == Graphic3d_TypeOfShadingModel_PhongFacet, "shading model drift");
static_assert(OCB_SHADING_GOURAUD  == Graphic3d_TypeOfShadingModel_Gouraud,   "shading model drift");
static_assert(OCB_SHADING_PHONG    == Graphic3d_TypeOfShadingModel_Phong,     "shading model drift");
static_assert(OCB_SHADING_REALISTIC == Graphic3d_TypeOfShadingModel_Pbr,      "shading model drift");
static_assert(OCB_SHADING_SHADOW   == Graphic3d_TypeOfShadingModel_PbrFacet,  "shading model drift");

static_assert(OCB_GRID_RECTANGULAR == Aspect_GT_Rectangular, "grid type drift");
static_assert(OCB_GRID_CIRCULAR    == Aspect_GT_Circular,    "grid type drift");
static_assert(OCB_GRID_LINES       == Aspect_GDM_Lines,      "grid draw mode drift");
static_assert(OCB_GRID_POINTS      == Aspect_GDM_Points,     "grid draw mode drift");
static_assert(Aspect_TOHM_COLOR    == 0, "highlight method drift");
static_assert(Aspect_TOHM_BOUNDBOX == 1, "highlight method drift");
// OcbHighlightType uses its own stable ordering; Prs3d_TypeOfHighlight_Selected
// is 1 and _LocalSelected is 3, so the values intentionally do NOT match and
// ocb_ais_ctx_set_highlight_style maps them explicitly.
} // namespace

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

using AisObjHandle = Handle(AIS_InteractiveObject);
using AisCtxHandle = Handle(AIS_InteractiveContext);
using ViewerHandle = Handle(V3d_Viewer);
using ViewHandle = Handle(V3d_View);
using DrawerHandle = Handle(Prs3d_Drawer);
using ColorToolHandle = Handle(XCAFDoc_ColorTool);
using LabelHandle = TDF_Label;
using LightHandle = Handle(Graphic3d_CLight);
using CameraHandle = Handle(Graphic3d_Camera);

HandleTable& table() {
    return HandleTable::instance();
}

Quantity_Color toColor(OcbColor c) {
    return Quantity_Color(c.r, c.g, c.b, Quantity_TOC_RGB);
}

OcbColor fromColor(const Quantity_Color& c) {
    return OcbColor{static_cast<float>(c.Red()),
                    static_cast<float>(c.Green()),
                    static_cast<float>(c.Blue())};
}

AisObjHandle* asAisObj(OcbAisObj obj) {
    return table().get<AisObjHandle>(obj);
}

AisCtxHandle* asCtx(OcbAisContext ctx) {
    return table().get<AisCtxHandle>(ctx);
}

ViewHandle* asView(OcbView view) {
    return table().get<ViewHandle>(view);
}

OcbStatus requireCtx(OcbAisContext ctx) {
    if (!asCtx(ctx) || !(*asCtx(ctx))) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid context handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    return OCB_OK;
}

OcbStatus requireObj(OcbAisObj obj) {
    if (!asAisObj(obj) || !(*asAisObj(obj))) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    return OCB_OK;
}

bool toShadingModel(int model, Graphic3d_TypeOfShadingModel& out) {
    switch (model) {
    case OCB_SHADING_COLOR:     out = Graphic3d_TypeOfShadingModel_Unlit;      return true;
    case OCB_SHADING_FLAT:      out = Graphic3d_TypeOfShadingModel_PhongFacet; return true;
    case OCB_SHADING_GOURAUD:   out = Graphic3d_TypeOfShadingModel_Gouraud;    return true;
    case OCB_SHADING_PHONG:     out = Graphic3d_TypeOfShadingModel_Phong;      return true;
    case OCB_SHADING_REALISTIC: out = Graphic3d_TypeOfShadingModel_Pbr;        return true;
    case OCB_SHADING_SHADOW:    out = Graphic3d_TypeOfShadingModel_PbrFacet;   return true;
    // OCCT 7.7 has no enum value 6; VERTEX is an alias of Gouraud.
    case OCB_SHADING_VERTEX:    out = Graphic3d_TypeOfShadingModel_Gouraud;    return true;
    default:                    return false;
    }
}

// OcbHighlightType is an independent stable ordering, not a mirror of
// Prs3d_TypeOfHighlight (Selected=1, Dynamic=2, LocalSelected=3, LocalDynamic=4).
bool toHighlightType(int type, Prs3d_TypeOfHighlight& out) {
    switch (type) {
    case OCB_HIGHLIGHT_LOCAL_SELECTED: out = Prs3d_TypeOfHighlight_LocalSelected; return true;
    case OCB_HIGHLIGHT_LOCAL_DYNAMIC:  out = Prs3d_TypeOfHighlight_LocalDynamic;  return true;
    case OCB_HIGHLIGHT_SELECTED:       out = Prs3d_TypeOfHighlight_Selected;      return true;
    case OCB_HIGHLIGHT_DYNAMIC:        out = Prs3d_TypeOfHighlight_Dynamic;       return true;
    default:                           return false;
    }
}

bool toHighlightMethod(int method, Aspect_TypeOfHighlightMethod& out) {
    switch (method) {
    case 0: out = Aspect_TOHM_COLOR;    return true;
    case 1: out = Aspect_TOHM_BOUNDBOX; return true;
    default: return false;
    }
}

} // namespace

extern "C" {

OcbStatus OCB_CALL ocb_geom_new_cartesian_point(double x, double y, double z, OcbGeom* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(Handle(Geom_CartesianPoint)(new Geom_CartesianPoint(x, y, z)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_geom_release(OcbGeom geom) {
    table().release(geom);
}

OcbStatus OCB_CALL ocb_ais_new_point(OcbGeom geom, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    auto* g = table().get<Handle(Geom_CartesianPoint)>(geom);
    if (!g || g->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid geom handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(Handle(AIS_InteractiveObject)(new AIS_Point(*g)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_new_xcaf_object(OcbLabel label, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(Handle(AIS_InteractiveObject)(new XCAFPrs_AISObject(*lab)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_color(OcbAisObj obj, OcbColor color) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        (*asAisObj(obj))->SetColor(toColor(color));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_material(OcbAisObj obj, int material) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        (*asAisObj(obj))->SetMaterial(static_cast<Graphic3d_NameOfMaterial>(material));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_display_mode(OcbAisObj obj, int mode) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        (*asAisObj(obj))->SetDisplayMode(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_transparency(OcbAisObj obj, double value) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        (*asAisObj(obj))->SetTransparency(static_cast<Standard_Real>(value));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_marker(OcbAisObj obj, int type) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        Handle(AIS_Point) point = Handle(AIS_Point)::DownCast(*asAisObj(obj));
        if (point.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "marker only supported on AIS_Point");
            return OCB_ERR_INVALID_HANDLE;
        }
        point->SetMarker(static_cast<Aspect_TypeOfMarker>(type));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_zlayer(OcbAisObj obj, int layer) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        (*asAisObj(obj))->SetZLayer(static_cast<Graphic3d_ZLayerId>(layer));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_zlayer(OcbAisObj obj, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = static_cast<int>((*asAisObj(obj))->ZLayer());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_has_color(OcbAisObj obj, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = (*asAisObj(obj))->HasColor();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_get_color(OcbAisObj obj, OcbColor* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        Quantity_Color color;
        (*asAisObj(obj))->Color(color);
        *out = fromColor(color);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_transparency(OcbAisObj obj, double* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = (*asAisObj(obj))->Transparency();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_material(OcbAisObj obj, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = static_cast<int>((*asAisObj(obj))->Material());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_attributes(OcbAisObj obj, OcbDrawer* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = table().add((*asAisObj(obj))->Attributes());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_downcast_shape(OcbAisObj obj, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(*h);
        if (shape.IsNull()) return OCB_ERR_INVALID_HANDLE;
        *out = table().add(AisObjHandle(shape));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_point_get_position(OcbAisObj obj, OcbPnt* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(AIS_Point) point = Handle(AIS_Point)::DownCast(*h);
        if (point.IsNull()) {
            setError(OCB_ERR_INVALID_ARG, "object is not AIS_Point");
            return OCB_ERR_INVALID_ARG;
        }
        const Handle(Geom_Point)& comp = point->Component();
        if (comp.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "point component is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        const gp_Pnt p = comp->Pnt();
        *out = OcbPnt{ p.X(), p.Y(), p.Z() };
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_point_set_position(OcbAisObj obj, OcbPnt p) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(AIS_Point) point = Handle(AIS_Point)::DownCast(*h);
        if (point.IsNull()) {
            setError(OCB_ERR_INVALID_ARG, "object is not AIS_Point");
            return OCB_ERR_INVALID_ARG;
        }
        point->SetComponent(new Geom_CartesianPoint(gp_Pnt(p.x, p.y, p.z)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_downcast_point(OcbAisObj obj, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(AIS_Point) point = Handle(AIS_Point)::DownCast(*h);
        if (point.IsNull()) return OCB_ERR_INVALID_HANDLE;
        *out = table().add(AisObjHandle(point));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_shape_shape(OcbAisObj obj, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(*h);
        if (shape.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "object is not AIS_Shape");
            return OCB_ERR_INVALID_HANDLE;
        }
        *out = table().add(shape->Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_has_transformation(OcbAisObj obj, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        *out = (*asAisObj(obj))->HasTransformation();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_local_transformation(OcbAisObj obj, OcbTrsf* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        const gp_Trsf trsf = (*asAisObj(obj))->LocalTransformation();
        for (int row = 1; row <= 3; ++row) {
            for (int col = 1; col <= 4; ++col) {
                out->v[(row - 1) * 4 + (col - 1)] = trsf.Value(row, col);
            }
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_set_local_transformation(OcbAisObj obj, OcbTrsf trsf) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        gp_Trsf t;
        t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                    trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                    trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
        if (t.Form() == gp_Identity) return OCB_OK;
        (*asAisObj(obj))->SetLocalTransformation(TopLoc_Location(t));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_xcaf_get_label(OcbAisObj obj, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(XCAFPrs_AISObject) xcaf = Handle(XCAFPrs_AISObject)::DownCast(*h);
        if (xcaf.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "object is not XCAFPrs_AISObject");
            return OCB_ERR_INVALID_HANDLE;
        }
        const TDF_Label& label = xcaf->GetLabel();
        if (label.IsNull()) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "no label bound");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = table().add(label);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_line_aspect_new(OcbColor color, double width, OcbLineAspect* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(Handle(Prs3d_LineAspect)(
            new Prs3d_LineAspect(toColor(color), Aspect_TOL_SOLID, static_cast<Standard_Real>(width))));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_line_aspect_release(OcbLineAspect la) {
    table().release(la);
}

void OCB_CALL ocb_drawer_release(OcbDrawer drawer) {
    table().release(drawer);
}

OcbStatus OCB_CALL ocb_drawer_set_face_boundary_draw(OcbDrawer drawer, bool enable) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    if (!d || d->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->SetFaceBoundaryDraw(enable);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_drawer_set_free_boundary_draw(OcbDrawer drawer, bool enable) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    if (!d || d->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->SetFreeBoundaryDraw(enable);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_drawer_set_face_boundary_aspect(OcbDrawer drawer, OcbLineAspect aspect) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    Handle(Prs3d_LineAspect)* la = table().get<Handle(Prs3d_LineAspect)>(aspect);
    if (!d || d->IsNull() || !la || la->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer/line aspect handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->SetFaceBoundaryAspect(*la);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_drawer_set_polygon_offsets(OcbDrawer drawer, int mode, float factor, float units) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    if (!d || d->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(Prs3d_ShadingAspect) shading = (*d)->ShadingAspect();
        if (shading.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "drawer has no shading aspect");
            return OCB_ERR_OCC_EXCEPTION;
        }
        shading->Aspect()->SetPolygonOffsets(mode, factor, units);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 独占研究对象:让 drawer 拥有自己的 ShadingAspect,避免改到全局共通外观 */
OCB_API OcbStatus OCB_CALL ocb_drawer_setup_own_shading_aspect(OcbDrawer drawer) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    if (!d || d->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->SetupOwnShadingAspect();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ---------------- Viewer / Context ---------------- */

OcbStatus OCB_CALL ocb_viewer_create(void* hwnd, OcbViewer* out) {
    if (!out || !hwnd) {
        setError(OCB_ERR_INVALID_ARG, "out or hwnd is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        static Handle(Aspect_DisplayConnection) display;
        static Handle(OpenGl_GraphicDriver) driver;
        if (driver.IsNull()) {
            display = new Aspect_DisplayConnection();
            driver = new OpenGl_GraphicDriver(display);
        }
#ifdef _WIN32
        Handle(WNT_Window) window = new WNT_Window(reinterpret_cast<HWND>(hwnd));
#else
        setError(OCB_ERR_NOT_IMPLEMENTED, "platform not supported yet");
        return OCB_ERR_NOT_IMPLEMENTED;
#endif
        ViewerHandle viewer = new V3d_Viewer(driver);
        // 灯光不由这里配置：V3d_Viewer::SetLightOff() 无参重载在灯表非空时有崩溃风险
        // （其内部走已废弃的成员迭代器），由消费方用 ocb_light_* + add_light + lights_on
        // 自行管理（桥的 lights_off/on 已改为遍历拷贝逐灯激活/去激活）。
        out[0] = table().add(viewer);
        // 创建主视图，绑定窗口
        (void)window;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_context(OcbViewer viewer, OcbAisContext* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(Handle(AIS_InteractiveContext)(new AIS_InteractiveContext(*v)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_active_view(OcbViewer viewer, OcbView* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if ((*v)->ActiveViews().IsEmpty()) {
            setError(OCB_ERR_OCC_EXCEPTION, "no active view");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add((*v)->ActiveViews().First());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_viewer_release(OcbViewer viewer) {
    table().release(viewer);
}

OcbStatus OCB_CALL ocb_ais_ctx_display(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Display(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_display_ex(OcbAisContext ctx, OcbAisObj obj, int mode, int priority, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Display(*asAisObj(obj), mode, priority, update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_erase(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Erase(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_remove(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Remove(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_update_viewer(OcbAisContext ctx) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->UpdateCurrentViewer();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_update(OcbAisContext ctx, OcbAisObj obj, bool updateViewer) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Update(*asAisObj(obj), updateViewer);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_wrap_native(void* nativeObj, OcbAisObj* out) {
    if (!out || !nativeObj) {
        setError(OCB_ERR_INVALID_ARG, "native object or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        AIS_InteractiveObject* obj = static_cast<AIS_InteractiveObject*>(nativeObj);
        *out = table().add(AisObjHandle(obj));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_displayed_count(OcbAisContext ctx, int* out) {    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        AIS_ListOfInteractive lst;
        (*asCtx(ctx))->DisplayedObjects(lst);
        *out = lst.Extent();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_displayed_at(OcbAisContext ctx, int index, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        AIS_ListOfInteractive lst;
        (*asCtx(ctx))->DisplayedObjects(lst);
        if (index < 0 || index >= lst.Extent()) {
            setError(OCB_ERR_INVALID_ARG, "index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        AIS_ListIteratorOfListOfInteractive it(lst);
        int i = 0;
        for (; it.More(); it.Next(), ++i) {
            if (i == index) {
                *out = table().add(AisObjHandle(it.Value()));
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_is_displayed(OcbAisContext ctx, OcbAisObj obj, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        *out = ((*asCtx(ctx))->DisplayStatus(*asAisObj(obj)) == AIS_DS_Displayed);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_clear_selected(OcbAisContext ctx, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->ClearSelected(update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_activate(OcbAisContext ctx, int mode) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->Activate(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_deactivate(OcbAisContext ctx) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->Deactivate();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_deactivate_mode(OcbAisContext ctx, int mode) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        (*asCtx(ctx))->Deactivate(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_selection_mode_active(
        OcbAisContext ctx, OcbAisObj obj, int mode, bool active) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        if (active) {
            (*asCtx(ctx))->Activate(*asAisObj(obj), mode);
        } else {
            (*asCtx(ctx))->Deactivate(*asAisObj(obj), mode);
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_color(OcbAisContext ctx, OcbAisObj obj, OcbColor color, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->SetColor(*asAisObj(obj), toColor(color), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_material(OcbAisContext ctx, OcbAisObj obj, int material, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->SetMaterial(*asAisObj(obj), static_cast<Graphic3d_NameOfMaterial>(material), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_transparency(OcbAisContext ctx, OcbAisObj obj, double value, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->SetTransparency(*asAisObj(obj), static_cast<Standard_Real>(value), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_unset_transparency(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*asCtx(ctx))->UnsetTransparency(*h, update ? Standard_True : Standard_False);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_display_priority(OcbAisContext ctx, OcbAisObj obj, int priority) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->SetDisplayPriority(*asAisObj(obj), priority);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_erase_all(OcbAisContext ctx, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->EraseAll(update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_clear_detected(OcbAisContext ctx, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->ClearDetected(update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_is_valid(OcbAisContext ctx, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    AisCtxHandle* h = asCtx(ctx);
    *out = (h != nullptr && !h->IsNull());
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_ais_ctx_remove_all(OcbAisContext ctx, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->RemoveAll(update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_move_to(OcbAisContext ctx, OcbView view, int x, int y, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    try {
        (*asCtx(ctx))->MoveTo(x, y, *v, update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_has_detected(OcbAisContext ctx, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        *out = (*asCtx(ctx))->HasDetected();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_detected(OcbAisContext ctx, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(AIS_InteractiveObject)& detected = (*asCtx(ctx))->DetectedInteractive();
        if (detected.IsNull()) {
            *out = OCB_INVALID;
            return OCB_OK;
        }
        *out = table().add(detected);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_detected_native(OcbAisContext ctx, void** out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(AIS_InteractiveObject)& detected = (*asCtx(ctx))->DetectedInteractive();
        *out = detected.IsNull() ? nullptr : (void*)detected.get();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_selected_native(OcbAisContext ctx, void** out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(AIS_InteractiveObject)& selected = (*asCtx(ctx))->SelectedInteractive();
        *out = selected.IsNull() ? nullptr : (void*)selected.get();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_native(OcbAisContext ctx, void** out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(SelectMgr_EntityOwner)& owner = (*asCtx(ctx))->SelectedOwner();
        *out = owner.IsNull() ? nullptr : (void*)owner.get();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_displayed_at_native(OcbAisContext ctx, int index, void** out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        AIS_ListOfInteractive lst;
        (*asCtx(ctx))->DisplayedObjects(lst);
        if (index < 0 || index >= lst.Extent()) {
            setError(OCB_ERR_INVALID_ARG, "index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        AIS_ListIteratorOfListOfInteractive it(lst);
        int i = 0;
        for (; it.More(); it.Next(), ++i) {
            if (i == index) {
                *out = it.Value().IsNull() ? nullptr : (void*)it.Value().get();
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_selected_owner(OcbAisContext ctx, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(SelectMgr_EntityOwner)& owner = (*asCtx(ctx))->SelectedOwner();
        if (owner.IsNull()) {
            *out = OCB_INVALID;
            return OCB_OK;
        }
        *out = table().add(owner);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_select_detected(OcbAisContext ctx, int mode) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->SelectDetected(static_cast<AIS_SelectionScheme>(mode));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_select_rectangle(OcbAisContext ctx, OcbView view,
    int x1, int y1, int x2, int y2, int scheme, int* outStatus) {
    if (!outStatus) {
        setError(OCB_ERR_INVALID_ARG, "outStatus is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const AIS_StatusOfPick pickSt = (*asCtx(ctx))->SelectRectangle(
            Graphic3d_Vec2i(x1, y1), Graphic3d_Vec2i(x2, y2), *v,
            static_cast<AIS_SelectionScheme>(scheme));
        *outStatus = static_cast<int>(pickSt);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_pixel_tolerance(OcbAisContext ctx, float tolerance) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        (*asCtx(ctx))->SetPixelTolerance(tolerance);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_display_mode(OcbAisContext ctx, int mode, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        (*asCtx(ctx))->SetDisplayMode(static_cast<AIS_DisplayMode>(mode), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_automatic_hilight(OcbAisContext ctx, bool enabled) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        (*asCtx(ctx))->SetAutomaticHilight(enabled);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_default_deflection(OcbAisContext ctx, int type,
    double maximalChordalDeviation, double deviationAngleRad) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        const Handle(Prs3d_Drawer)& drawer = (*asCtx(ctx))->DefaultDrawer();
        if (drawer.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "default drawer is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        drawer->SetTypeOfDeflection(static_cast<Aspect_TypeOfDeflection>(type));
        drawer->SetMaximalChordialDeviation(maximalChordalDeviation);
        drawer->SetDeviationAngle(deviationAngleRad);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_selection_style(OcbAisContext ctx, OcbColor color,
    float transparency, int zlayer) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        const Handle(Prs3d_Drawer)& drawer = (*asCtx(ctx))->SelectionStyle();
        if (drawer.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "selection drawer is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        drawer->SetColor(toColor(color));
        drawer->SetTransparency(transparency);
        drawer->SetZLayer(static_cast<Graphic3d_ZLayerId>(zlayer));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_highlight_color(OcbAisContext ctx, OcbColor color) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        const Handle(Prs3d_Drawer)& drawer = (*asCtx(ctx))->HighlightStyle();
        if (drawer.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "highlight drawer is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        drawer->SetColor(toColor(color));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_init_selected(OcbAisContext ctx) {    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->InitSelected();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_more_selected(OcbAisContext ctx, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        *out = (*asCtx(ctx))->MoreSelected();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_next_selected(OcbAisContext ctx) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        (*asCtx(ctx))->NextSelected();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_selected(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->SetSelected(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_add_or_remove_selected(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->AddOrRemoveSelected(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_selected(OcbAisContext ctx, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(AIS_InteractiveObject)& selected = (*asCtx(ctx))->SelectedInteractive();
        if (selected.IsNull()) {
            *out = OCB_INVALID;
            return OCB_OK;
        }
        *out = table().add(selected);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_picked_point(OcbAisContext ctx, int index, OcbPnt* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        const Handle(StdSelect_ViewerSelector3d)& sel = (*asCtx(ctx))->MainSelector();
        if (sel.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "main selector is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        const gp_Pnt p = sel->PickedPoint(index);
        *out = OcbPnt{ p.X(), p.Y(), p.Z() };
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_location(OcbAisContext ctx, OcbAisObj obj, OcbTrsf trsf) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        gp_Trsf t;
        t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                    trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                    trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
        (*asCtx(ctx))->SetLocation(*asAisObj(obj), TopLoc_Location(t));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_redisplay(OcbAisContext ctx, OcbAisObj obj, bool update) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    const OcbStatus st2 = requireObj(obj);
    if (st2 != OCB_OK) return st2;
    try {
        (*asCtx(ctx))->Redisplay(*asAisObj(obj), update);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_ais_ctx_release(OcbAisContext ctx) {
    table().release(ctx);
}

/* ---------------- 视图 ---------------- */

OcbStatus OCB_CALL ocb_view_fit_all(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->FitAll();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_zfit_all(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->ZFitAll();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_redraw(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->Redraw();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_redraw_immediate(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->RedrawImmediate();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_invalidate(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->Invalidate();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_bg_gradient(OcbView view, OcbColor top, OcbColor bottom, int method) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetBgGradientColors(toColor(top), toColor(bottom),
                                  static_cast<Aspect_GradientFillMethod>(method));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_convert_with_proj(OcbView view, int x, int y,
    double* xp, double* yp, double* zp, double* vx, double* vy, double* vz) {
    if (!xp || !yp || !zp || !vx || !vy || !vz) {
        setError(OCB_ERR_INVALID_ARG, "output pointer is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Standard_Real Xp = 0, Yp = 0, Zp = 0, Vx = 0, Vy = 0, Vz = 0;
        (*v)->ConvertWithProj(x, y, Xp, Yp, Zp, Vx, Vy, Vz);
        *xp = Xp; *yp = Yp; *zp = Zp; *vx = Vx; *vy = Vy; *vz = Vz;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_create(OcbViewer viewer, void* hwnd, OcbView* out) {
    if (!out || !hwnd) {
        setError(OCB_ERR_INVALID_ARG, "out or hwnd is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        ViewHandle view = (*v)->CreateView();
        if (view.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "CreateView returned null");
            return OCB_ERR_OCC_EXCEPTION;
        }
#ifdef _WIN32
        Handle(WNT_Window) window = new WNT_Window(reinterpret_cast<HWND>(hwnd));
        if (window->IsVirtual()) {
            setError(OCB_ERR_OCC_EXCEPTION, "window is virtual (no native handle)");
            return OCB_ERR_OCC_EXCEPTION;
        }
        view->SetWindow(window);
        if (!window->IsMapped()) {
            window->Map();
        }
#else
        setError(OCB_ERR_NOT_IMPLEMENTED, "platform not supported yet");
        return OCB_ERR_NOT_IMPLEMENTED;
#endif
        out[0] = table().add(view);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_must_be_resized(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->MustBeResized();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

namespace {
V3d_TypeOfOrientation projTypeOf(int t) {
    switch (t) {
    case OCB_PROJ_TOP:        return V3d_Zpos;
    case OCB_PROJ_FRONT:      return V3d_Yneg;
    case OCB_PROJ_RIGHT:      return V3d_Xpos;
    case OCB_PROJ_AXO_RIGHT:  return V3d_XposYnegZpos;
    case OCB_PROJ_ISO:
    default:                  return V3d_XnegYnegZpos;
    }
}
}  // namespace

OcbStatus OCB_CALL ocb_view_set_proj_type(OcbView view, int v3dOrientation) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->SetProj(static_cast<V3d_TypeOfOrientation>(v3dOrientation)); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_proj(OcbView view, int projType) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetProj(projTypeOf(projType));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_dump(OcbView view, const char* path, int width, int height) {
    if (!path || !path[0]) {
        setError(OCB_ERR_INVALID_ARG, "path is null/empty");
        return OCB_ERR_INVALID_ARG;
    }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // 说明：OCCT 7.7 的 Aspect_Window 不再提供 SetSize，无法临时改视口尺寸；
        // 此处按当前窗口尺寸出图（width/height 保留给上层接口兼容，暂不生效）。
        (void)width;
        (void)height;
        (*v)->Redraw();
        (*v)->Dump(path);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_to_rgb32(OcbView view, int width, int height,
    unsigned char* buffer, int bufferSize, int* outWidth, int* outHeight) {
    if (!buffer || !outWidth || !outHeight || width <= 0 || height <= 0) {
        setError(OCB_ERR_INVALID_ARG, "invalid buffer/dimension");
        return OCB_ERR_INVALID_ARG;
    }
    if (bufferSize < width * height * 4) {
        setError(OCB_ERR_INVALID_ARG, "buffer too small");
        return OCB_ERR_INVALID_ARG;
    }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Image_AlienPixMap pix;
        if (!(*v)->ToPixMap(pix, width, height, Graphic3d_BT_RGB, Standard_True)) {
            setError(OCB_ERR_OCC_EXCEPTION, "ToPixMap failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        for (Standard_Integer y = 0; y < (Standard_Integer)pix.Height(); ++y) {
            unsigned char* row = buffer + (size_t)y * (size_t)pix.Width() * 4;
            for (Standard_Integer x = 0; x < (Standard_Integer)pix.Width(); ++x) {
                const Quantity_ColorRGBA c = pix.PixelColor(x, y);
                const Quantity_Color& rgb = c.GetRGB();
                row[x * 4 + 0] = (unsigned char)(rgb.Blue() * 255.0);
                row[x * 4 + 1] = (unsigned char)(rgb.Green() * 255.0);
                row[x * 4 + 2] = (unsigned char)(rgb.Red() * 255.0);
                row[x * 4 + 3] = 0xFF;
            }
        }
        *outWidth = (int)pix.Width();
        *outHeight = (int)pix.Height();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_trihedron(OcbView view, bool show) {    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (show) {
            (*v)->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.08, V3d_ZBUFFER);
        }
        else {
            (*v)->TriedronErase();
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_view_release(OcbView view) {
    table().release(view);
}

OcbStatus OCB_CALL ocb_view_wrap_native(void* nativeView, OcbView* out) {
    if (!out || !nativeView) {
        setError(OCB_ERR_INVALID_ARG, "native view or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        V3d_View* v = static_cast<V3d_View*>(nativeView);
        *out = table().add(ViewHandle(v));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void* OCB_CALL ocb_view_native_ptr(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) return nullptr;
    return (void*)(*v).get();
}

void* OCB_CALL ocb_viewer_native_ptr(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) return nullptr;
    return (void*)(*v).get();
}

OcbStatus OCB_CALL ocb_view_project_point(OcbView view, double x, double y, double z,
    int* outSx, int* outSy) {
    if (!outSx || !outSy) { setError(OCB_ERR_INVALID_ARG, "out is null"); return OCB_ERR_INVALID_ARG; }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try {
        const Handle(Aspect_Window)& win = (*v)->Window();
        if (!win.IsNull() && !win->IsMapped()) {
            // 未显示的窗口（缩略图等）：视图尺寸无效，投影无意义
            setError(OCB_ERR_NOT_READY, "view window is not mapped");
            return OCB_ERR_NOT_READY;
        }
        (*v)->MustBeResized();
        Standard_Real sx = 0, sy = 0;
        (*v)->Project(x, y, z, sx, sy);
        // Project 返回“以视图中心为原点”的坐标（X 右正、Y 上正）；
        // move_to/select_rectangle 需要“左上角为原点”的像素坐标 → 平移半宽、翻转 Y
        Standard_Integer w = 0, h = 0;
        win->Size(w, h);
        *outSx = (int)(w / 2.0 + sx);
        *outSy = (int)(h / 2.0 - sy);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_start_rotation(OcbView view, int x, int y) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->StartRotation(x, y); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_rotation(OcbView view, int x, int y) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->Rotation(x, y); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_pan(OcbView view, double dx, double dy) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->Pan(dx, dy); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_start_zoom_at_point(OcbView view, int x, int y) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->StartZoomAtPoint(x, y); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_zoom_at_point(OcbView view, int x, int y, double dx, double dy) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->ZoomAtPoint(x, y, dx, dy); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_rendering_params(OcbView view,
    int transparencyMethod, int method, bool shadow, bool antialiasing,
    int raytracingDepth, int toneMapping, int msaa) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try {
        Graphic3d_RenderingParams& p = (*v)->ChangeRenderingParams();
        p.TransparencyMethod = static_cast<Graphic3d_RenderTransparentMethod>(transparencyMethod);
        p.Method = static_cast<Graphic3d_RenderingMode>(method);
        p.IsShadowEnabled = shadow;
        p.IsAntialiasingEnabled = antialiasing;
        p.RaytracingDepth = raytracingDepth;
        p.ToneMappingMethod = static_cast<Graphic3d_ToneMappingMethod>(toneMapping);
        p.NbMsaaSamples = msaa;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_trihedron_ex(OcbView view, bool show, OcbColor color, double scale) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try {
        if (show) (*v)->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, toColor(color), scale, V3d_ZBUFFER);
        else (*v)->TriedronErase();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_wrap_native(void* nativeViewer, OcbViewer* out) {
    if (!out || !nativeViewer) {
        setError(OCB_ERR_INVALID_ARG, "nativeViewer or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        V3d_Viewer* v = static_cast<V3d_Viewer*>(nativeViewer);
        *out = table().add(ViewerHandle(v));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_fit_all_margin(OcbView view, double margin) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid view handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->FitAll(margin, Standard_False); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_deactivate_grid(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try { (*v)->DeactivateGrid(); return OCB_OK; }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_set_z_up_plane(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try {
        (*v)->SetPrivilegedPlane(gp_Ax3(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(0., 1., 0.)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_setup_rect_grid(OcbViewer viewer,
    double step, double graphicSize, OcbColor echoColor, bool activate) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle"); return OCB_ERR_INVALID_HANDLE; }
    clearError();
    try {
        Handle(Graphic3d_AspectMarker3d) marker =
            new Graphic3d_AspectMarker3d(Aspect_TOM_O_PLUS, toColor(echoColor), 1.0);
        (*v)->SetGridEcho(marker);
        (*v)->SetRectangularGridGraphicValues(graphicSize, graphicSize, 0.0);
        (*v)->SetRectangularGridValues(0, 0, step, step, 0.0);
        if (activate) (*v)->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_selected_owner_shape(OcbAisContext ctx, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    try {
        const Handle(SelectMgr_EntityOwner)& owner = (*asCtx(ctx))->SelectedOwner();
        if (owner.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "no selection owner");
            return OCB_ERR_INVALID_HANDLE;
        }
        const Handle(StdSelect_BRepOwner) brepOwner =
            Handle(StdSelect_BRepOwner)::DownCast(owner);
        if (brepOwner.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "owner is not a topology owner");
            return OCB_ERR_INVALID_HANDLE;
        }
        *out = table().add(brepOwner->Shape());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_wrap_native(void* nativeCtx, OcbAisContext* out) {
    if (!out || !nativeCtx) {
        setError(OCB_ERR_INVALID_ARG, "null native context or out");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        AIS_InteractiveContext* ctx = static_cast<AIS_InteractiveContext*>(nativeCtx);
        *out = table().add(AisCtxHandle(ctx));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void* OCB_CALL ocb_ais_native_ptr(OcbAisObj obj) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) return nullptr;
    return (void*)(*h).get();
}

void* OCB_CALL ocb_ais_ctx_native_ptr(OcbAisContext ctx) {
    AisCtxHandle* h = asCtx(ctx);
    if (!h || h->IsNull()) return nullptr;
    return (void*)(*h).get();
}

void* OCB_CALL ocb_shape_native_ptr(OcbShape shape) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!s) return nullptr;
    return (void*)s;
}

OcbStatus OCB_CALL ocb_shape_wrap_native(void* nativeShape, OcbShape* out) {
    if (!out || !nativeShape) {
        setError(OCB_ERR_INVALID_ARG, "null native shape or out");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        TopoDS_Shape* raw = static_cast<TopoDS_Shape*>(nativeShape);
        if (raw->IsNull()) {
            setError(OCB_ERR_INVALID_ARG, "native shape is null");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(TopoDS_Shape(*raw));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_point_aspect_new(int markerType, OcbColor color, double size, OcbPointAspect* out) {
    if (!out) { setError(OCB_ERR_INVALID_ARG, "out is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        *out = table().add(Handle(Prs3d_PointAspect)(new Prs3d_PointAspect(
            static_cast<Aspect_TypeOfMarker>(markerType),
            Quantity_Color(color.r, color.g, color.b, Quantity_TOC_RGB),
            static_cast<Standard_Real>(size))));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_point_aspect_release(OcbPointAspect aspect) {
    table().release(aspect);
}

OcbStatus OCB_CALL ocb_drawer_set_point_aspect(OcbDrawer drawer, OcbPointAspect aspect) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    Handle(Prs3d_PointAspect)* a = table().get<Handle(Prs3d_PointAspect)>(aspect);
    if (!d || !a || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer/aspect handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    (*d)->SetPointAspect(*a);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_drawer_set_line_aspect(OcbDrawer drawer, OcbLineAspect aspect) {
    DrawerHandle* d = table().get<DrawerHandle>(drawer);
    Handle(Prs3d_LineAspect)* a = table().get<Handle(Prs3d_LineAspect)>(aspect);
    if (!d || !a || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid drawer/aspect handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    (*d)->SetLineAspect(*a);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_light_new_ambient(float r, float g, float b, float intensity, OcbLight* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Handle(Graphic3d_CLight) light =
            Handle(Graphic3d_CLight)(new Graphic3d_CLight(Graphic3d_TypeOfLightSource_Ambient));
        light->SetColor(Quantity_Color(r, g, b, Quantity_TOC_RGB));
        light->SetIntensity(intensity);
        *out = table().add(light);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_light_new_directional(
    float r, float g, float b, float intensity, bool headlight,
    double dx, double dy, double dz, OcbLight* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Handle(Graphic3d_CLight) light =
            Handle(Graphic3d_CLight)(new Graphic3d_CLight(Graphic3d_TypeOfLightSource_Directional));
        light->SetColor(Quantity_Color(r, g, b, Quantity_TOC_RGB));
        light->SetIntensity(intensity);
        light->SetHeadlight(headlight ? Standard_True : Standard_False);
        light->SetDirection(gp_Dir(dx, dy, dz));
        *out = table().add(light);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_light_release(OcbLight light) {
    table().release(light);
}

OcbStatus OCB_CALL ocb_viewer_add_light(OcbViewer viewer, OcbLight light) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    LightHandle* l = table().get<LightHandle>(light);
    if (!l || l->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid light handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->AddLight(*l);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_lights_on(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // 不用无参 SetLightOn()：其内部走已废弃的成员迭代器，灯表非空时可致崩溃；
        // 改为对 DefinedLights 拷贝逐灯激活（激活会把灯从 DefinedLights 挪到 ActiveLights，
        // 因此必须遍历拷贝，避免迭代器失效）
        NCollection_List<Handle(Graphic3d_CLight)> pending = (*v)->DefinedLights();
        for (V3d_ListOfLightIterator it(pending); it.More(); it.Next()) {
            (*v)->SetLightOn(it.Value());
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_lights_off(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // 同 lights_on：逐灯 Deactivate（遍历 ActiveLights 拷贝）
        NCollection_List<Handle(Graphic3d_CLight)> pending = (*v)->ActiveLights();
        for (V3d_ListOfLightIterator it(pending); it.More(); it.Next()) {
            (*v)->SetLightOff(it.Value());
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ------------------------------------------------------------------
 * v1.3.0 视图状态
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_view_reset(OcbView view) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->Reset(Standard_True);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_shading_model(OcbView view, int model) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    Graphic3d_TypeOfShadingModel shadingModel = Graphic3d_TypeOfShadingModel_DEFAULT;
    if (!toShadingModel(model, shadingModel)) {
        setError(OCB_ERR_INVALID_ARG, "invalid shading model");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        (*v)->ChangeRenderingParams().ShadingModel = shadingModel;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_immediate_update(OcbView view, bool on) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetImmediateUpdate(on ? Standard_True : Standard_False);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_set_computed_mode(
    OcbViewer viewer, bool computed, bool defaultComputed) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetComputedMode(computed ? Standard_True : Standard_False);
        (*v)->SetDefaultComputedMode(defaultComputed ? Standard_True : Standard_False);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_set_default_lights(OcbViewer viewer) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetDefaultLights();
        // Avoid the deprecated no-arg SetLightOn() iterator, which is unsafe
        // when the defined-light list is non-empty. Activate a snapshot instead.
        NCollection_List<Handle(Graphic3d_CLight)> pending = (*v)->DefinedLights();
        for (V3d_ListOfLightIterator it(pending); it.More(); it.Next()) {
            (*v)->SetLightOn(it.Value());
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_set_default_view_size(OcbViewer viewer, double size) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetDefaultViewSize(size);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ------------------------------------------------------------------
 * v1.3.0 相机
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_view_get_camera(OcbView view, OcbCamera* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const Handle(Graphic3d_Camera)& camera = (*v)->Camera();
        if (camera.IsNull()) {
            setError(OCB_ERR_INTERNAL, "view camera is null");
            return OCB_ERR_INTERNAL;
        }
        *out = table().add(CameraHandle(new Graphic3d_Camera(camera)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_set_camera(OcbView view, OcbCamera cam) {
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    CameraHandle* camera = table().get<CameraHandle>(cam);
    if (!camera || camera->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid camera handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetCamera(*camera);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_camera_copy(OcbCamera src, OcbCamera* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    CameraHandle* camera = table().get<CameraHandle>(src);
    if (!camera || camera->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid camera handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(CameraHandle(new Graphic3d_Camera(*camera)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_camera_lerp(OcbCamera a, OcbCamera b, double t, OcbCamera* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    CameraHandle* cameraA = table().get<CameraHandle>(a);
    CameraHandle* cameraB = table().get<CameraHandle>(b);
    if (!cameraA || cameraA->IsNull() || !cameraB || cameraB->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid camera handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // OCCT 7.7's Interpolate() dereferences the output handle, so allocate
        // the result camera before calling it.
        Handle(Graphic3d_Camera) result = new Graphic3d_Camera();
        Graphic3d_Camera::Interpolate(*cameraA, *cameraB, t, result);
        *out = table().add(result);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_camera_release(OcbCamera cam) {
    table().release(cam);
}

/* ------------------------------------------------------------------
 * v1.3.0 AIS 高亮/选择样式 / TextLabel / ConnectedInteractive
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_ais_ctx_set_highlight_style(OcbAisContext ctx, int type,
    int method, OcbColor color, int displayMode, float transparency) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    Prs3d_TypeOfHighlight highlightType = Prs3d_TypeOfHighlight_Dynamic;
    if (!toHighlightType(type, highlightType)) {
        setError(OCB_ERR_INVALID_ARG, "invalid highlight type");
        return OCB_ERR_INVALID_ARG;
    }
    Aspect_TypeOfHighlightMethod highlightMethod = Aspect_TOHM_COLOR;
    if (!toHighlightMethod(method, highlightMethod)) {
        setError(OCB_ERR_INVALID_ARG, "invalid highlight method");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        // Modify the existing drawer in place; replacing it would drop settings
        // that were configured through earlier calls.
        const Handle(Prs3d_Drawer)& drawer =
            (*asCtx(ctx))->HighlightStyle(highlightType);
        if (drawer.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "highlight drawer is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        drawer->SetColor(toColor(color));
        drawer->SetMethod(highlightMethod);
        drawer->SetDisplayMode(displayMode);
        drawer->SetTransparency(transparency);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_ctx_set_selection_style_ex(OcbAisContext ctx,
    OcbColor color, int displayMode, float transparency, int zlayer) {
    const OcbStatus st = requireCtx(ctx);
    if (st != OCB_OK) return st;
    clearError();
    try {
        const Handle(Prs3d_Drawer)& drawer = (*asCtx(ctx))->SelectionStyle();
        if (drawer.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "selection drawer is null");
            return OCB_ERR_INVALID_HANDLE;
        }
        drawer->SetColor(toColor(color));
        drawer->SetDisplayMode(displayMode);
        drawer->SetTransparency(transparency);
        drawer->SetZLayer(static_cast<Graphic3d_ZLayerId>(zlayer));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_textlabel_new(const char* utf8, OcbAisObj* out) {
    if (!utf8 || !out) {
        setError(OCB_ERR_INVALID_ARG, "utf8 or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Handle(AIS_TextLabel) label = new AIS_TextLabel();
        label->SetText(TCollection_ExtendedString(utf8, true));
        *out = table().add(AisObjHandle(label));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_textlabel_set_text(OcbAisObj obj, const char* utf8) {
    if (!utf8) {
        setError(OCB_ERR_INVALID_ARG, "utf8 is null");
        return OCB_ERR_INVALID_ARG;
    }
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        Handle(AIS_TextLabel) label = Handle(AIS_TextLabel)::DownCast(*asAisObj(obj));
        if (label.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "object is not an AIS_TextLabel");
            return OCB_ERR_INVALID_HANDLE;
        }
        label->SetText(TCollection_ExtendedString(utf8, true));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_textlabel_set_color(OcbAisObj obj, OcbColor color) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        Handle(AIS_TextLabel) label = Handle(AIS_TextLabel)::DownCast(*asAisObj(obj));
        if (label.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "object is not an AIS_TextLabel");
            return OCB_ERR_INVALID_HANDLE;
        }
        label->SetColor(toColor(color));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_textlabel_set_transform_persistence(
    OcbAisObj obj, int mode, int corner, int x, int y) {
    const OcbStatus st = requireObj(obj);
    if (st != OCB_OK) return st;
    try {
        Handle(AIS_TextLabel) label = Handle(AIS_TextLabel)::DownCast(*asAisObj(obj));
        if (label.IsNull()) {
            setError(OCB_ERR_INVALID_HANDLE, "object is not an AIS_TextLabel");
            return OCB_ERR_INVALID_HANDLE;
        }
        // OCCT 7.7 has (mode, corner, Graphic3d_Vec2i); the (mode, gp_Pnt)
        // overload only accepts Zoom/Rotate persistence and would throw here.
        Handle(Graphic3d_TransformPers) persistence = new Graphic3d_TransformPers(
            static_cast<Graphic3d_TransModeFlags>(mode),
            static_cast<Aspect_TypeOfTriedronPosition>(corner),
            Graphic3d_Vec2i(x, y));
        label->SetTransformPersistence(persistence);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_new_connected(OcbShape shape, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    const TopoDS_Shape* src = table().get<TopoDS_Shape>(shape);
    if (!src || src->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // Connect() takes an AIS_InteractiveObject, not a TopoDS_Shape.
        Handle(AIS_ConnectedInteractive) connected = new AIS_ConnectedInteractive();
        Handle(AIS_Shape) reference = new AIS_Shape(*src);
        connected->Connect(reference);
        *out = table().add(AisObjHandle(connected));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ------------------------------------------------------------------
 * v1.5.0 AIS_Trihedron（原点 / 局部三轴）
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_ais_trihedron_new(OcbAisObj* out) {
    if (!out) { setError(OCB_ERR_INVALID_ARG, "out is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        Handle(AIS_Trihedron) t = new AIS_Trihedron(new Geom_Axis2Placement(gp::XOY()));
        *out = table().add(AisObjHandle(t));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_datum_display_mode(OcbAisObj obj, int mode) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetDatumDisplayMode(static_cast<Prs3d_DatumMode>(mode));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_draw_arrows(OcbAisObj obj, bool on) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetDrawArrows(on);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_datum_part_color(OcbAisObj obj, int part, OcbColor color) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetDatumPartColor(static_cast<Prs3d_DatumParts>(part), toColor(color));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_label(OcbAisObj obj, int part, const char* utf8) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    if (!utf8) { setError(OCB_ERR_INVALID_ARG, "utf8 is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetLabel(static_cast<Prs3d_DatumParts>(part),
                    TCollection_ExtendedString(utf8, true));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_text_color(OcbAisObj obj, OcbColor color) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetTextColor(toColor(color));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_size(OcbAisObj obj, double size) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetSize(size);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_infinite_state(OcbAisObj obj, bool on) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetInfiniteState(on);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_line_style(OcbAisObj obj, int part, double width, int typeOfLine) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        Handle(Prs3d_DatumAspect) da = t->Attributes()->DatumAspect();
        if (da.IsNull()) { setError(OCB_ERR_INVALID_ARG, "datum aspect is null"); return OCB_ERR_INVALID_ARG; }
        Handle(Prs3d_LineAspect) la = da->LineAspect(static_cast<Prs3d_DatumParts>(part));
        if (la.IsNull()) { setError(OCB_ERR_INVALID_ARG, "line aspect is null"); return OCB_ERR_INVALID_ARG; }
        la->SetWidth(width);
        la->SetTypeOfLine(static_cast<Aspect_TypeOfLine>(typeOfLine));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_trihedron_set_zoom_persistence(OcbAisObj obj) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_Trihedron) t = Handle(AIS_Trihedron)::DownCast(*h);
    if (t.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_Trihedron"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        t->SetTransformPersistence(
            new Graphic3d_TransformPers(Graphic3d_TMF_ZoomPers, gp_Pnt(0.0, 0.0, 0.0)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ------------------------------------------------------------------
 * v1.5.0 AIS_ViewCube（导航立方体）
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_ais_viewcube_new(OcbAisObj* out) {
    if (!out) { setError(OCB_ERR_INVALID_ARG, "out is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        Handle(AIS_ViewCube) v = new AIS_ViewCube();
        *out = table().add(AisObjHandle(v));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_colors(OcbAisObj obj, OcbColor box, OcbColor inner, OcbColor text) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetBoxColor(toColor(box));
        v->SetInnerColor(toColor(inner));
        v->SetTextColor(toColor(text));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_size(OcbAisObj obj, double size) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetSize(size);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_axes_labels(OcbAisObj obj, const char* x, const char* y, const char* z) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    if (!x || !y || !z) { setError(OCB_ERR_INVALID_ARG, "axis label is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetAxesLabels(TCollection_AsciiString(x), TCollection_AsciiString(y), TCollection_AsciiString(z));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_axes_radius(OcbAisObj obj, double radius) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetAxesRadius(radius);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_transparency(OcbAisObj obj, double transparency) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetTransparency(transparency);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_decor(OcbAisObj obj, bool drawAxes, bool drawEdges,
    double boxEdgeGap, double axesPadding, double boxEdgeMinSize) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetDrawAxes(drawAxes);
        v->SetDrawEdges(drawEdges);
        v->SetBoxEdgeGap(boxEdgeGap);
        v->SetAxesPadding(axesPadding);
        v->SetBoxEdgeMinSize(boxEdgeMinSize);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_font(OcbAisObj obj, const char* family, int height) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    if (!family) { setError(OCB_ERR_INVALID_ARG, "font family is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetFont(TCollection_AsciiString(family));
        v->SetFontHeight(static_cast<double>(height));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_box_side_label(OcbAisObj obj, int faceId, const char* utf8) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    if (!utf8) { setError(OCB_ERR_INVALID_ARG, "label is null"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetBoxSideLabel(static_cast<V3d_TypeOfOrientation>(faceId),
                           TCollection_AsciiString(utf8));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_viewcube_set_triedron_pers(OcbAisObj obj, int corner, int x, int y) {
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) { setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle"); return OCB_ERR_INVALID_HANDLE; }
    Handle(AIS_ViewCube) v = Handle(AIS_ViewCube)::DownCast(*h);
    if (v.IsNull()) { setError(OCB_ERR_INVALID_ARG, "object is not AIS_ViewCube"); return OCB_ERR_INVALID_ARG; }
    clearError();
    try {
        v->SetTransformPersistence(
            new Graphic3d_TransformPers(Graphic3d_TMF_TriedronPers,
                                        static_cast<Aspect_TypeOfTriedronPosition>(corner),
                                        Graphic3d_Vec2i(x, y)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* ------------------------------------------------------------------
 * v1.3.0 网格
 * ------------------------------------------------------------------ */

OcbStatus OCB_CALL ocb_viewer_set_grid_echo_marker(
    OcbViewer viewer, int style, OcbColor color, double scale) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(Graphic3d_AspectMarker3d) marker = new Graphic3d_AspectMarker3d(
            static_cast<Aspect_TypeOfMarker>(style), toColor(color), scale);
        (*v)->SetGridEcho(marker);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_set_rect_grid_values(OcbViewer viewer,
    double xOrigin, double yOrigin, double xStep, double yStep, double rotation) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->SetRectangularGridValues(xOrigin, yOrigin, xStep, yStep, rotation);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_activate_grid(
    OcbViewer viewer, int gridType, int drawMode) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (gridType != OCB_GRID_RECTANGULAR && gridType != OCB_GRID_CIRCULAR) {
        setError(OCB_ERR_INVALID_ARG, "invalid grid type");
        return OCB_ERR_INVALID_ARG;
    }
    if (drawMode != OCB_GRID_LINES && drawMode != OCB_GRID_POINTS) {
        setError(OCB_ERR_INVALID_ARG, "invalid grid draw mode");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        (*v)->ActivateGrid(
            static_cast<Aspect_GridType>(gridType),
            static_cast<Aspect_GridDrawMode>(drawMode));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_view_convert_to_grid(
    OcbView view, int x, int y, double* gx, double* gy, double* gz) {
    if (!gx || !gy || !gz) {
        setError(OCB_ERR_INVALID_ARG, "grid output is null");
        return OCB_ERR_INVALID_ARG;
    }
    ViewHandle* v = asView(view);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid view handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*v)->ConvertToGrid(x, y, *gx, *gy, *gz);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_viewer_show_grid_echo(
    OcbViewer viewer, OcbView view, double x, double y, double z) {
    ViewerHandle* v = table().get<ViewerHandle>(viewer);
    if (!v || v->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid viewer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    // OCCT 7.7 signature: ShowGridEcho(const Handle(V3d_View)&, const Graphic3d_Vertex&).
    ViewHandle* vw = asView(view);
    if (!vw || vw->IsNull()) {
        setError(OCB_ERR_INVALID_ARG, "invalid view handle");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        (*v)->ShowGridEcho(*vw, Graphic3d_Vertex(x, y, z));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

} // extern "C"

OcbStatus OCB_CALL ocb_ais_pick_probe(OcbAisObj obj, int* outNbSensitive0,
    int* outZLayer, int* outDisplayMode, OcbBox* outBBox) {
    if (!outNbSensitive0 || !outZLayer || !outDisplayMode || !outBBox) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    *outNbSensitive0 = -1; *outZLayer = -1; *outDisplayMode = -1; outBBox->valid = false;
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const Handle(SelectMgr_Selection)& sel = (*h)->Selection(0);
        if (!sel.IsNull()) *outNbSensitive0 = sel->Entities().Length();
        *outZLayer = (*h)->ZLayer();
        *outDisplayMode = (*h)->DisplayMode();
        // Fill the shape bounding box so consumers can compare it against the
        // label shape box (diagnostic probes rely on this).
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(*h);
        if (!aisShape.IsNull()) {
            Bnd_Box bbox;
            BRepBndLib::Add(aisShape->Shape(), bbox);
            if (!bbox.IsVoid()) {
                double x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0;
                bbox.Get(x1, y1, z1, x2, y2, z2);
                outBBox->xmin = x1; outBBox->ymin = y1; outBBox->zmin = z1;
                outBBox->xmax = x2; outBBox->ymax = y2; outBBox->zmax = z2;
                outBBox->valid = true;
            }
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_selection_probe(OcbAisObj obj, int* outHasMode0,
    int* outMode0Empty, int* outActiveModeCount) {
    if (!outHasMode0 || !outMode0Empty || !outActiveModeCount) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    *outHasMode0 = 0; *outMode0Empty = 1; *outActiveModeCount = 0;
    AisObjHandle* h = asAisObj(obj);
    if (!h || h->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid ais object handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        int active = 0;
        for (int mode = 0; mode <= 7; ++mode) {
            if (!(*h)->HasSelection(mode)) continue;
            ++active;
            if (mode == 0) {
                *outHasMode0 = 1;
                const Handle(SelectMgr_Selection)& sel = (*h)->Selection(0);
                *outMode0Empty = (!sel.IsNull() && sel->IsEmpty()) ? 1 : 0;
            }
        }
        *outActiveModeCount = active;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}
