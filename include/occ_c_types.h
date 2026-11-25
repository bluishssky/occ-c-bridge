// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#ifndef OCC_C_TYPES_H
#define OCC_C_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#  ifdef OCB_EXPORTS
#    define OCB_API __declspec(dllexport)
#  else
#    define OCB_API __declspec(dllimport)
#  endif
#  define OCB_CALL __cdecl
#else
#  define OCB_API __attribute__((visibility("default")))
#  define OCB_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------
 * 值类型：纯 POD，与 OpenCASCADE 无关
 * ------------------------------------------------------------------ */

typedef struct OcbPnt {
    double x, y, z;
} OcbPnt;

typedef struct OcbVec {
    double x, y, z;
} OcbVec;

typedef struct OcbDir {
    double dx, dy, dz;
} OcbDir;

typedef struct OcbAx2 {
    OcbPnt origin;
    OcbDir normal;
    OcbDir xdir;
} OcbAx2;

typedef struct OcbTrsf {
    double v[12]; /* 3x4 变换矩阵，row-major */
} OcbTrsf;

typedef struct OcbBox {
    double xmin, ymin, zmin;
    double xmax, ymax, zmax;
    bool   valid;
} OcbBox;

typedef struct OcbColor {
    float r, g, b;
} OcbColor;

/* 渲染/选择枚举镜像（值取自 OCCT 7.7，桥内 static_assert 校验） */
enum OcbRenderMode { OCB_RM_RASTERIZATION = 0 };
enum OcbTransparencyMethod { OCB_RTM_BLEND_OIT = 1 };
enum OcbToneMapping { OCB_TONE_DISABLED = 0, OCB_TONE_FILMIC = 1 };
enum OcbAisDisplayMode { OCB_AIS_WIREFRAME = 0, OCB_AIS_SHADED = 1 };
enum OcbSelectionScheme {
    OCB_SEL_SCHEME_UNKNOWN = -1,
    OCB_SEL_SCHEME_REPLACE = 0,
    OCB_SEL_SCHEME_ADD = 1,
    OCB_SEL_SCHEME_REMOVE = 2,
    OCB_SEL_SCHEME_XOR = 3,
    OCB_SEL_SCHEME_CLEAR = 4
};

/* 等价 V3d_TypeOfOrientation（顺序与 OCCT 一致，桥内 static_assert 校验）。
   可直接传给 ocb_view_set_proj_type(view, int)。 */
enum OcbOrientation {
    OCB_ORIENT_XPOS = 0,
    OCB_ORIENT_YPOS,
    OCB_ORIENT_ZPOS,
    OCB_ORIENT_XNEG,
    OCB_ORIENT_YNEG,
    OCB_ORIENT_ZNEG,
    OCB_ORIENT_XPOSYPOS,
    OCB_ORIENT_XPOSZPOS,
    OCB_ORIENT_YPOSZPOS,
    OCB_ORIENT_XNEGYNEG,
    OCB_ORIENT_XNEGYPOS,
    OCB_ORIENT_XNEGZNEG,
    OCB_ORIENT_XNEGZPOS,
    OCB_ORIENT_YNEGZNEG,
    OCB_ORIENT_YNEGZPOS,
    OCB_ORIENT_XPOSYNEG,
    OCB_ORIENT_XPOSZNEG,
    OCB_ORIENT_YPOSZNEG,
    OCB_ORIENT_XPOSYPOSZPOS,
    OCB_ORIENT_XPOSYNEGZPOS,
    OCB_ORIENT_XPOSYPOSZNEG,
    OCB_ORIENT_XNEGYPOSZPOS,
    OCB_ORIENT_XPOSYNEGZNEG,
    OCB_ORIENT_XNEGYPOSZNEG,
    OCB_ORIENT_XNEGYNEGZPOS,
    OCB_ORIENT_XNEGYNEGZNEG
};

/* ------------------------------------------------------------------
 * 渲染常量（值取自 OpenCASCADE 7.7；桥内部有 static_assert 校验，
 *  升级 OCCT 若枚举变化会在编译期报错，不会静默错位）
 * ------------------------------------------------------------------ */

/* 等价 Graphic3d_ZLayerId */
enum OcbZLayerId {
    OCB_ZLAYER_UNKNOWN = -1,
    OCB_ZLAYER_DEFAULT = 0,
    OCB_ZLAYER_TOP     = -2,
    OCB_ZLAYER_TOPMOST = -3,
    OCB_ZLAYER_TOPOSD  = -4,
    OCB_ZLAYER_BOTOSD  = -5
};

/* 等价 Graphic3d_DisplayPriority */
enum OcbDisplayPriority {
    OCB_DISPLAY_PRIORITY_INVALID  = -1,
    OCB_DISPLAY_PRIORITY_BOTTOM   = 0,
    OCB_DISPLAY_PRIORITY_BELOW    = 4,
    OCB_DISPLAY_PRIORITY_NORMAL   = 5,
    OCB_DISPLAY_PRIORITY_ABOVE    = 6,
    OCB_DISPLAY_PRIORITY_HIGHLIGHT = 9,
    OCB_DISPLAY_PRIORITY_TOPMOST  = 10
};

/* 等价 Graphic3d_NameOfMaterial（顺序与 OCCT 一致，桥内 static_assert 校验） */
enum OcbMaterial {
    OCB_MATERIAL_BRASS = 0,
    OCB_MATERIAL_BRONZE,
    OCB_MATERIAL_COPPER,
    OCB_MATERIAL_GOLD,
    OCB_MATERIAL_PEWTER,
    OCB_MATERIAL_PLASTERED,
    OCB_MATERIAL_PLASTIC,      /* Graphic3d_NameOfMaterial_Plastified */
    OCB_MATERIAL_SILVER,
    OCB_MATERIAL_STEEL,
    OCB_MATERIAL_STONE,
    OCB_MATERIAL_SHINY_PLASTIC,
    OCB_MATERIAL_SATIN,
    OCB_MATERIAL_METALIZED,
    OCB_MATERIAL_NEON_GNC,     /* Graphic3d_NameOfMaterial_Ionized */
    OCB_MATERIAL_CHROME,
    OCB_MATERIAL_ALUMINUM,
    OCB_MATERIAL_OBSIDIAN,
    OCB_MATERIAL_NEON,
    OCB_MATERIAL_JADE,
    OCB_MATERIAL_CHARCOAL,
    OCB_MATERIAL_WATER,
    OCB_MATERIAL_GLASS,
    OCB_MATERIAL_DIAMOND,
    OCB_MATERIAL_TRANSPARENT,
    OCB_MATERIAL_DEFAULT,
    OCB_MATERIAL_USER_DEFINED
};

/* 等价 TDF_Attribute 类型分类（ocb_label_attribute_kind_at 返回值）。
   只暴露稳定语义类别，不泄露 OCC 类名，保持消费方"源码零 OCC"。 */
typedef enum OcbAttrKind {
    OCB_ATTR_OTHER    = 0,
    OCB_ATTR_NAME     = 1,   /* TDataStd_Name */
    OCB_ATTR_INT      = 2,   /* TDataStd_Integer */
    OCB_ATTR_REAL     = 3,   /* TDataStd_Real */
    OCB_ATTR_SHAPE    = 4,   /* TNaming_NamedShape / TDataStd_Shape 类 */
    OCB_ATTR_LOCATION = 5    /* TDataStd_Location */
} OcbAttrKind;

typedef struct OcbQuat {
    double x, y, z, w;
} OcbQuat;

/* v1.3.0 C API 稳定数值（实现处显式映射到 OCCT 枚举） */
enum OcbHighlightType {
    OCB_HIGHLIGHT_LOCAL_SELECTED = 0,
    OCB_HIGHLIGHT_LOCAL_DYNAMIC  = 1,
    OCB_HIGHLIGHT_SELECTED       = 2,
    OCB_HIGHLIGHT_DYNAMIC        = 3
};

enum OcbGridType     { OCB_GRID_RECTANGULAR = 0, OCB_GRID_CIRCULAR = 1 };
enum OcbGridDrawMode { OCB_GRID_LINES = 0, OCB_GRID_POINTS = 1 };

enum OcbShadingModel {
    OCB_SHADING_COLOR = 0,
    OCB_SHADING_FLAT = 1,
    OCB_SHADING_GOURAUD = 2,
    OCB_SHADING_PHONG = 3,
    OCB_SHADING_REALISTIC = 4,
    OCB_SHADING_SHADOW = 5,
    OCB_SHADING_VERTEX = 6
};

/* ------------------------------------------------------------------
 * 不透明句柄：由 occ-c-bridge 内部管理生命周期
 * 0 表示无效句柄
 * ------------------------------------------------------------------ */

typedef uint64_t OcbShape;
typedef uint64_t OcbEdge;
typedef uint64_t OcbFace;
typedef uint64_t OcbWire;
typedef uint64_t OcbLabel;
typedef uint64_t OcbShapeTool;
typedef uint64_t OcbAisObj;
typedef uint64_t OcbAisContext;
typedef uint64_t OcbView;
typedef uint64_t OcbViewer;
typedef uint64_t OcbDoc;
typedef uint64_t OcbApp;
typedef uint64_t OcbGeom;
typedef uint64_t OcbMesh;
typedef uint64_t OcbDrawer;
typedef uint64_t OcbLineAspect;
typedef uint64_t OcbGProps;
typedef uint64_t OcbCls;
typedef uint64_t OcbExplorer;
typedef uint64_t OcbCurve;
typedef uint64_t OcbTri;
typedef uint64_t OcbStepReader;
typedef uint64_t OcbStepWriter;
typedef uint64_t OcbColorTool;
typedef uint64_t OcbShadingAspect;
typedef uint64_t OcbFillAspect;
typedef uint64_t OcbFillet;
typedef uint64_t OcbChamfer;
typedef uint64_t OcbLight;
typedef uint64_t OcbPointAspect;
typedef uint64_t OcbCamera;          /* Graphic3d_Camera 深拷贝句柄，必须 ocb_camera_release */

#define OCB_INVALID ((uint64_t)0)

/* ------------------------------------------------------------------
 * 状态码
 * ------------------------------------------------------------------ */

typedef enum OcbStatus {
    OCB_OK = 0,
    OCB_ERR_INVALID_HANDLE = 1,
    OCB_ERR_INVALID_ARG = 2,
    OCB_ERR_OCC_EXCEPTION = 3,
    OCB_ERR_NOT_IMPLEMENTED = 4,
    OCB_ERR_IO = 5,
    OCB_ERR_NOT_READY = 7,
    OCB_ERR_INTERNAL = 6
} OcbStatus;

#ifdef __cplusplus
} /* extern "C" */

static_assert(sizeof(OcbPnt) == 24, "OcbPnt layout mismatch");
static_assert(sizeof(OcbVec) == 24, "OcbVec layout mismatch");
static_assert(sizeof(OcbTrsf) == 96, "OcbTrsf layout mismatch");
#endif

#endif /* OCC_C_TYPES_H */
