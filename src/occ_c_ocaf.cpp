// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "occ_c_bridge.h"
#include "handle_table.h"

// OpenCASCADE（只允许出现在中间层内部）
#include <AIS_Shape.hxx>
#include <BinXCAFDrivers.hxx>
#include <PCDM_ReaderStatus.hxx>
#include <PCDM_StoreStatus.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_Tool.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Real.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_ExtendedString.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Failure.hxx>

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

using DocApp = Handle(TDocStd_Application);
using DocHandle = Handle(TDocStd_Document);
using ShapeToolHandle = Handle(XCAFDoc_ShapeTool);
using ColorToolHandle = Handle(XCAFDoc_ColorTool);

Quantity_Color toColor(OcbColor c) {
    return Quantity_Color(c.r, c.g, c.b, Quantity_TOC_RGB);
}

HandleTable& table() {
    return HandleTable::instance();
}

/* 把 OCC 属性类名折叠成稳定枚举；不向调用方暴露类名字符串。 */
int classifyAttrKind(const char* typeName) {
    if (!typeName) return OCB_ATTR_OTHER;
    const std::string name(typeName);
    if (name == "TDataStd_Name") return OCB_ATTR_NAME;
    if (name == "TDataStd_Integer") return OCB_ATTR_INT;
    if (name == "TDataStd_Real") return OCB_ATTR_REAL;
    if (name == "TDataStd_Location") return OCB_ATTR_LOCATION;
    if (name == "TNaming_NamedShape" || name.rfind("TDataStd_Shape", 0) == 0) {
        return OCB_ATTR_SHAPE;
    }
    return OCB_ATTR_OTHER;
}

} // namespace

extern "C" {

OcbStatus OCB_CALL ocb_app_new(OcbApp* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        DocApp app = new TDocStd_Application();
        BinXCAFDrivers::DefineFormat(app);
        *out = table().add(app);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_app_new_document(OcbApp app, const char* format, OcbDoc* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocApp* a = table().get<DocApp>(app);
    if (!a || !format || !(*a)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid app handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        DocHandle doc;
        (*a)->NewDocument(format, doc);
        if (doc.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "NewDocument returned null");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(doc);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_app_close(OcbApp app, OcbDoc doc) {
    DocApp* a = table().get<DocApp>(app);
    DocHandle* d = table().get<DocHandle>(doc);
    if (!a || !d || !(*a) || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid app/doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*a)->Close(*d);
        table().release(doc);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_app_release(OcbApp app) {
    table().release(app);
}

OcbStatus OCB_CALL ocb_doc_new_command(OcbDoc doc) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->NewCommand();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_commit_command(OcbDoc doc) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->CommitCommand();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_undo(OcbDoc doc, bool* ok) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (ok) *ok = false;
    clearError();
    try {
        const bool done = (*d)->Undo();
        if (ok) *ok = done;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_redo(OcbDoc doc, bool* ok) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    if (ok) *ok = false;
    clearError();
    try {
        const bool done = (*d)->Redo();
        if (ok) *ok = done;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_get_undo_limit(OcbDoc doc, int* out) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!out || !d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*d)->GetUndoLimit();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_doc_main_label(OcbDoc doc, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add((*d)->Main());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_save(OcbApp app, OcbDoc doc, const char* path) {
    DocApp* a = table().get<DocApp>(app);
    DocHandle* d = table().get<DocHandle>(doc);
    if (!a || !(*a) || !d || !(*d) || !path) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid app/doc handle or path");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const PCDM_StoreStatus rc = (*a)->SaveAs(*d, TCollection_ExtendedString(path, Standard_True));
        if (rc != PCDM_SS_OK) {
            setError(OCB_ERR_IO, "SaveAs failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_open(OcbApp app, const char* path, OcbDoc* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocApp* a = table().get<DocApp>(app);
    if (!a || !(*a) || !path) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid app handle or path");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        DocHandle doc;
        const PCDM_ReaderStatus rc = (*a)->Open(TCollection_ExtendedString(path, Standard_True), doc);
        if (rc != PCDM_RS_OK) {
            const std::string msg = std::string("Open failed, reader status=") +
                                    std::to_string(static_cast<int>(rc));
            setError(OCB_ERR_IO, msg.c_str());
            return OCB_ERR_IO;
        }
        if (doc.IsNull()) {
            setError(OCB_ERR_IO, "Open returned null document");
            return OCB_ERR_IO;
        }
        *out = table().add(doc);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_set_undo_limit(OcbDoc doc, int limit) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->SetUndoLimit(limit);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_doc_release(OcbDoc doc) {
    table().release(doc);
}

OcbStatus OCB_CALL ocb_shapetool_get(OcbDoc doc, OcbShapeTool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(XCAFDoc_DocumentTool::ShapeTool((*d)->Main()));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 显式双旗标的 AddShape(默认桥接口固定 Standard_False,这里照顾业务级旗标) */
OcbStatus OCB_CALL ocb_shapetool_add_shape_ex(OcbShapeTool st, OcbShape shape,
    bool makeAssemblySimpleGraph, bool autoLabel, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!tool || !s || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_Label label = (*tool)->AddShape(*s, makeAssemblySimpleGraph, autoLabel);
        if (label.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "AddShape returned null label");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(label);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 子形状(GetSubShapes)数量与按序读取 */
OcbStatus OCB_CALL ocb_shapetool_get_sub_shape_count(OcbShapeTool st, OcbLabel label, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence subs;
        (*tool)->GetSubShapes(*lab, subs);
        *out = subs.Length();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_sub_shape(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence subs;
        (*tool)->GetSubShapes(*lab, subs);
        if (index < 0 || index >= subs.Length()) {
            setError(OCB_ERR_INVALID_ARG, "sub shape index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(subs.Value(index + 1));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_add_shape(OcbShapeTool st, OcbShape shape, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!tool || !s || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_Label label = (*tool)->AddShape(*s, Standard_False);
        if (label.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "AddShape returned null label");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(label);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_shape(OcbShapeTool st, OcbLabel label, OcbShape* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !lab || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TopoDS_Shape s = (*tool)->GetShape(*lab);
        if (s.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "label has no shape");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(std::move(s));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_set_shape(OcbShapeTool st, OcbLabel label, OcbShape shape) {
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!tool || !lab || !s || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label/shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*tool)->SetShape(*lab, *s);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_update(OcbShapeTool st) {
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    if (!tool || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*tool)->UpdateAssemblies();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_free_shape_count(OcbShapeTool st, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    if (!tool || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetFreeShapes(seq);
        *out = seq.Length();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_shapetool_release(OcbShapeTool st) {
    table().release(st);
}

OcbStatus OCB_CALL ocb_label_set_name(OcbLabel label, const char* name) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || !name) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle or name");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDataStd_Name::Set(*lab, TCollection_ExtendedString(name, Standard_True));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* TCollection_ExtendedString(UTF-16) → UTF-8 字节流。
   注意：不能用 static_cast<char>(码点)——那会把中文等 BMP 字符截断成单字节，
   产生无效 UTF-8（表现为树里名字乱码）。 */
static void extendedStringToUtf8(const TCollection_ExtendedString& str, std::string& out)
{
    out.clear();
    const int len = str.Length();
    out.reserve(static_cast<size_t>(len) * 3);
    for (int i = 1; i <= len; ++i) {
        unsigned int c = static_cast<unsigned int>(str.Value(i));
        if (c >= 0xD800u && c <= 0xDBFFu && i < len) {
            const unsigned int lo = static_cast<unsigned int>(str.Value(i + 1));
            if (lo >= 0xDC00u && lo <= 0xDFFFu) {
                c = 0x10000u + ((c - 0xD800u) << 10) + (lo - 0xDC00u);
                ++i;
            }
        }
        if (c < 0x80u) {
            out.push_back(static_cast<char>(c));
        }
        else if (c < 0x800u) {
            out.push_back(static_cast<char>(0xC0u | (c >> 6)));
            out.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        }
        else if (c < 0x10000u) {
            out.push_back(static_cast<char>(0xE0u | (c >> 12)));
            out.push_back(static_cast<char>(0x80u | ((c >> 6) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        }
        else {
            out.push_back(static_cast<char>(0xF0u | (c >> 18)));
            out.push_back(static_cast<char>(0x80u | ((c >> 12) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | ((c >> 6) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        }
    }
}

OcbStatus OCB_CALL ocb_label_get_name(OcbLabel label, char* buf, int bufSize) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || !buf || bufSize < 1) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle or buffer");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(TDataStd_Name) attr;
        if (!(*lab).FindAttribute(TDataStd_Name::GetID(), attr)) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "label has no name attribute");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        TCollection_ExtendedString str = attr->Get();
        std::string utf8;
        extendedStringToUtf8(str, utf8);
        const int n = (static_cast<int>(utf8.size()) < bufSize - 1)
            ? static_cast<int>(utf8.size()) : (bufSize - 1);
        if (n > 0) std::memcpy(buf, utf8.data(), static_cast<size_t>(n));
        buf[n] = '\0';
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_label_release(OcbLabel label) {
    table().release(label);
}

OcbStatus OCB_CALL ocb_label_walk_fathers_touch(OcbLabel label) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_Label parent = lab->Father();
        while (!parent.IsNull()) {
            Handle(TNaming_NamedShape) ns;
            if (parent.FindAttribute(TNaming_NamedShape::GetID(), ns)) {
                TNaming_Builder builder(parent);
                builder.Modify(ns->Get(), ns->Get());
            }
            parent = parent.Father();
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_ais_new_shape(OcbShape shape, OcbAisObj* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!s || s->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(Handle(AIS_InteractiveObject)(new AIS_Shape(*s)));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_ais_release(OcbAisObj obj) {
    table().release(obj);
}

/* ------------------------------------------------------------------
 * OCAF 扩充
 * ------------------------------------------------------------------ */

namespace {
OcbTrsf ocafTrsfToOcb(const gp_Trsf& trsf) {
    OcbTrsf r;
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col) {
            r.v[(row - 1) * 4 + (col - 1)] = trsf.Value(row, col);
        }
    }
    return r;
}
} // namespace

OcbStatus OCB_CALL ocb_label_get_shape(OcbLabel label, OcbShape* out) {
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
        TopoDS_Shape s;
        if (!XCAFDoc_ShapeTool::GetShape(*lab, s) || s.IsNull()) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "label has no shape");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = table().add(std::move(s));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* Label 自身的 location（等价 XCAFDoc_ShapeTool::GetLocation(label)）。
   注意与 ocb_label_get_shape 的区别：后者返回 GetShape 烘焙过 location 的 shape。 */
OcbStatus OCB_CALL ocb_label_get_location(OcbLabel label, OcbTrsf* out) {
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
        const TopLoc_Location loc = XCAFDoc_ShapeTool::GetLocation(*lab);
        *out = ocafTrsfToOcb(loc.Transformation());
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_is_equal(OcbLabel a, OcbLabel b, bool* out) {
    TDF_Label* la = table().get<TDF_Label>(a);
    TDF_Label* lb = table().get<TDF_Label>(b);
    if (!out || !la || !lb) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handles");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = la->IsEqual(*lb);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_label_is_null(OcbLabel label, bool* out) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!out || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = lab->IsNull();
    return OCB_OK;
}

/* 主动有效性查询:登记存在且标签非空 */
OcbStatus OCB_CALL ocb_label_wrap_native(const void* nativeLabel, OcbLabel* out) {
    if (!nativeLabel || !out) {
        setError(OCB_ERR_INVALID_ARG, "nativeLabel or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(*static_cast<const TDF_Label*>(nativeLabel));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_is_valid(OcbLabel label, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TDF_Label* lab = table().get<TDF_Label>(label);
    *out = (lab != nullptr && !lab->IsNull());
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_label_tag(OcbLabel label, int* out) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!out || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = lab->Tag();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_label_entry(OcbLabel label, char* buf, int bufSize) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!buf || bufSize < 1 || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle or buffer");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TCollection_AsciiString entry;
        TDF_Tool::Entry(*lab, entry);
        const int len = entry.Length();
        const int n = (len < bufSize - 1) ? len : bufSize - 1;
        for (int i = 0; i < n; ++i) buf[i] = entry.Value(i + 1);
        buf[n] = '\0';
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_child(OcbLabel parent, int tag, OcbLabel* out) {
    TDF_Label* lab = table().get<TDF_Label>(parent);
    if (!out || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid parent label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // Parent-relative lookup (TDF_Label::FindChild); TDF_Tool::Label resolves
        // against the document root and never creates, which is not what callers need.
        TDF_Label child = lab->FindChild(tag, false);
        if (child.IsNull()) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "child label not found");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = table().add(child);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_new_child(OcbLabel parent, int tag, OcbLabel* out) {
    TDF_Label* lab = table().get<TDF_Label>(parent);
    if (!out || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid parent label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        // Parent-relative with create=true (equivalent to TDF_Label::NewChild).
        TDF_Label child = lab->FindChild(tag, true);
        if (child.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "child label creation failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(child);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_father(OcbLabel label, OcbLabel* out) {
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
    *out = table().add(lab->Father());
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_label_set_real(OcbLabel label, double value) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDataStd_Real::Set(*lab, value);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_get_real(OcbLabel label, double* out) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!out || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Handle(TDataStd_Real) attr;
        if (!(*lab).FindAttribute(TDataStd_Real::GetID(), attr)) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "label has no real attribute");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = attr->Get();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_child_count(OcbLabel parent, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TDF_Label* lab = table().get<TDF_Label>(parent);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid parent label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        int n = 0;
        for (TDF_ChildIterator it(*lab, false); it.More(); it.Next()) ++n;
        *out = n;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_child_at(OcbLabel parent, int index, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    TDF_Label* lab = table().get<TDF_Label>(parent);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid parent label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (index < 0) {
            setError(OCB_ERR_INVALID_ARG, "child index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        int i = 0;
        for (TDF_ChildIterator it(*lab, false); it.More(); it.Next(), ++i) {
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

OcbStatus OCB_CALL ocb_label_has_child(OcbLabel label, bool* out) {
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
        TDF_ChildIterator it(*lab, false);
        *out = it.More();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_set_int(OcbLabel label, int value) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDataStd_Integer::Set(*lab, value);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_get_int(OcbLabel label, int* out) {
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
        Handle(TDataStd_Integer) attr;
        if (!lab->FindAttribute(TDataStd_Integer::GetID(), attr)) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "label has no integer attribute");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = attr->Get();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_attribute_count(OcbLabel label, int* out) {
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
        int n = 0;
        for (TDF_AttributeIterator it(*lab); it.More(); it.Next()) ++n;
        *out = n;
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_attribute_kind_at(OcbLabel label, int index, int* kindOut) {
    if (!kindOut) {
        setError(OCB_ERR_INVALID_ARG, "kindOut is null");
        return OCB_ERR_INVALID_ARG;
    }
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (index < 0) {
            setError(OCB_ERR_INVALID_ARG, "attribute index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        int i = 0;
        for (TDF_AttributeIterator it(*lab); it.More(); it.Next(), ++i) {
            if (i == index) {
                *kindOut = classifyAttrKind(it.Value()->DynamicType()->Name());
                return OCB_OK;
            }
        }
        setError(OCB_ERR_INVALID_ARG, "attribute index out of range");
        return OCB_ERR_INVALID_ARG;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_label_forget_attributes(OcbLabel label) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        /* false = 不清空子标签，只忘掉本标签自身属性 */
        lab->ForgetAllAttributes(false);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_doc_has_open_command(OcbDoc doc, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*d)->HasOpenCommand();
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_doc_abort_command(OcbDoc doc) {
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*d)->AbortCommand();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_new_shape(OcbShapeTool st, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    if (!tool || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_Label label = (*tool)->NewShape();
        if (label.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "NewShape returned null label");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(label);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_remove_shape(OcbShapeTool st, OcbLabel label) {
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*tool)->RemoveShape(*lab);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_is_reference(OcbShapeTool st, OcbLabel label, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*tool)->IsReference(*lab);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shapetool_get_referred_shape(OcbShapeTool st, OcbLabel label, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_Label referred;
        if (!(*tool)->GetReferredShape(*lab, referred)) {
            setError(OCB_ERR_NOT_IMPLEMENTED, "label is not a reference");
            return OCB_ERR_NOT_IMPLEMENTED;
        }
        *out = table().add(referred);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_is_component(OcbShapeTool st, OcbLabel label, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*tool)->IsComponent(*lab);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shapetool_is_assembly(OcbShapeTool st, OcbLabel label, bool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    *out = (*tool)->IsAssembly(*lab);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_shapetool_get_component_count(OcbShapeTool st, OcbLabel label, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetComponents(*lab, seq);
        *out = seq.Length();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_user_count(OcbShapeTool st, OcbLabel label, int* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetUsers(*lab, seq);
        *out = seq.Length();
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_user(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetUsers(*lab, seq);
        if (index < 0 || index >= seq.Length()) {
            setError(OCB_ERR_INVALID_ARG, "user index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(seq.Value(index + 1));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_component(OcbShapeTool st, OcbLabel label, int index, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetComponents(*lab, seq);
        if (index < 0 || index >= seq.Length()) {
            setError(OCB_ERR_INVALID_ARG, "component index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(seq.Value(index + 1));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_add_component(OcbShapeTool st, OcbLabel parent, OcbLabel child, OcbTrsf trsf, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* pl = table().get<TDF_Label>(parent);
    TDF_Label* cl = table().get<TDF_Label>(child);
    if (!tool || !(*tool) || !pl || !cl) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/parent/child handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        gp_Trsf t;
        t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                    trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                    trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
        TDF_Label label = (*tool)->AddComponent(*pl, *cl, t);
        if (label.IsNull()) {
            setError(OCB_ERR_OCC_EXCEPTION, "AddComponent returned null label");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(label);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 递归重建装配标签子树的 TNaming compound（自底向上）。
   背景：NewShape+AddComponent 建的装配，其父标签 TNaming 里存的 compound 恒为空
   （AddComponent 只建引用属性），GetShape 返回空，导致按标签导出 STEP 时写出空装配。 */
static void rebuildAssemblyShapeRec(XCAFDoc_ShapeTool* tool, const TDF_Label& L)
{
    // 先自底向上处理直接子引用的定义标签
    TDF_LabelSequence comps;
    if (tool->GetComponents(L, comps, Standard_False)) {
        for (TDF_LabelSequence::Iterator it(comps); it.More(); it.Next()) {
            TDF_Label ref;
            if (tool->GetReferredShape(it.Value(), ref)) {
                rebuildAssemblyShapeRec(tool, ref);
            }
        }
    }
    if (!tool->IsAssembly(L)) return;

    // 组合直接组件：GetShape(引用) = 定义形状 + 组件位置
    // （注意：GetComponents 是 Append 语义不清空入参，必须用独立序列，否则会重复叠加成员）
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);
    {
        TDF_LabelSequence directComps;
        if (tool->GetComponents(L, directComps, Standard_False)) {
            for (TDF_LabelSequence::Iterator it(directComps); it.More(); it.Next()) {
                TopoDS_Shape s;
                if (tool->GetShape(it.Value(), s) && !s.IsNull()) {
                    builder.Add(compound, s);
                }
            }
        }
    }
    // 关键：先移除旧 TNaming（空 compound 的初始演化），否则新旧两个演化并存时
    // STEPCAF 写装配会把每个组件写两条（一条有名、一条无名重复）
    L.ForgetAttribute(TNaming_NamedShape::GetID());
    TNaming_Builder bld(L);
    bld.Generated(compound);
}

// 装配"占位形体"：给装配标签写一个**空 compound**（非空以便 STEPCAF 的 Transfer 不跳过该产品，
// 但内部不含任何几何）。这样导出的 STEP 结构完整、几何只在零件层各存一份，
// 避免 rebuild_assembly_shapes 那种"每层都存一份合并几何"造成的文件膨胀与重复实体。
static void setAssemblyPlaceholderRec(XCAFDoc_ShapeTool* tool, const TDF_Label& L)
{
    TDF_LabelSequence comps;
    if (tool->GetComponents(L, comps, Standard_False)) {
        for (TDF_LabelSequence::Iterator it(comps); it.More(); it.Next()) {
            TDF_Label ref;
            if (tool->GetReferredShape(it.Value(), ref)) {
                setAssemblyPlaceholderRec(tool, ref);
            }
        }
    }
    if (!tool->IsAssembly(L)) return;
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);           // 空壳：不放任何子件几何
    L.ForgetAttribute(TNaming_NamedShape::GetID());
    TNaming_Builder bld(L);
    bld.Generated(compound);
}

OcbStatus OCB_CALL ocb_shapetool_set_assembly_placeholder(OcbShapeTool st, OcbLabel label) {
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        setAssemblyPlaceholderRec((*tool).get(), *lab);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_rebuild_assembly_shapes(OcbShapeTool st, OcbLabel label) {    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        rebuildAssemblyShapeRec((*tool).get(), *lab);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_remove_component(OcbShapeTool st, OcbLabel label) {
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*tool)->RemoveComponent(*lab);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_set_location(OcbShapeTool st, OcbLabel label, OcbTrsf trsf, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        gp_Trsf t;
        t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                    trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                    trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
        TDF_Label refLabel;
        if (!(*tool)->SetLocation(*lab, TopLoc_Location(t), refLabel)) {
            setError(OCB_ERR_OCC_EXCEPTION, "SetLocation failed");
            return OCB_ERR_OCC_EXCEPTION;
        }
        *out = table().add(refLabel);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_location(OcbShapeTool st, OcbLabel label, OcbTrsf* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!tool || !(*tool) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const TopLoc_Location loc = (*tool)->GetLocation(*lab);
        const gp_Trsf& t = loc.Transformation();
        for (int row = 1; row <= 3; ++row)
            for (int col = 1; col <= 4; ++col)
                out->v[(row - 1) * 4 + (col - 1)] = t.Value(row, col);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_shapetool_get_free_shape(OcbShapeTool st, int index, OcbLabel* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ShapeToolHandle* tool = table().get<ShapeToolHandle>(st);
    if (!tool || !(*tool)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shapetool handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        TDF_LabelSequence seq;
        (*tool)->GetFreeShapes(seq);
        if (index < 0 || index >= seq.Length()) {
            setError(OCB_ERR_INVALID_ARG, "free shape index out of range");
            return OCB_ERR_INVALID_ARG;
        }
        *out = table().add(seq.Value(index + 1));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_colortool_get(OcbDoc doc, OcbColorTool* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    DocHandle* d = table().get<DocHandle>(doc);
    if (!d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        *out = table().add(XCAFDoc_DocumentTool::ColorTool((*d)->Main()));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_colortool_set_color(OcbColorTool tool, OcbLabel label, OcbColor color, int colorType) {
    ColorToolHandle* ct = table().get<ColorToolHandle>(tool);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!ct || !(*ct) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid colortool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*ct)->SetColor(*lab, toColor(color),
                        static_cast<XCAFDoc_ColorType>(colorType));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 子形状着色(与 XCAFDoc_ColorTool::SetColor(shape,...) 语义一致,内部顺查 sub label) */
OcbStatus OCB_CALL ocb_colortool_set_shape_color(OcbColorTool tool, OcbShape shape,
    OcbColor color, int colorType) {
    ColorToolHandle* ct = table().get<ColorToolHandle>(tool);
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!ct || !(*ct) || !s) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid colortool/shape handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*ct)->SetColor(*s, toColor(color), static_cast<XCAFDoc_ColorType>(colorType));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_colortool_release(OcbColorTool tool) {
    table().release(tool);
}

/* 查询某标签在指定颜色通道的着色(与 XCAFDoc_ColorTool::GetColor 语义一致) */
OcbStatus OCB_CALL ocb_colortool_get_color(OcbColorTool tool, OcbLabel label,
    int colorType, bool* okOut, OcbColor* out) {
    if (!okOut || !out) {
        setError(OCB_ERR_INVALID_ARG, "ok/out is null");
        return OCB_ERR_INVALID_ARG;
    }
    ColorToolHandle* ct = table().get<ColorToolHandle>(tool);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!ct || !(*ct) || !lab) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid colortool/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        Quantity_Color c;
        const Standard_Boolean has = (*ct)->GetColor(*lab, static_cast<XCAFDoc_ColorType>(colorType), c);
        *okOut = has;
        if (has) {
            out->r = static_cast<float>(c.Red());
            out->g = static_cast<float>(c.Green());
            out->b = static_cast<float>(c.Blue());
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_xcaf_location_set(OcbLabel label, OcbTrsf trsf) {
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        gp_Trsf t;
        t.SetValues(trsf.v[0], trsf.v[1], trsf.v[2], trsf.v[3],
                    trsf.v[4], trsf.v[5], trsf.v[6], trsf.v[7],
                    trsf.v[8], trsf.v[9], trsf.v[10], trsf.v[11]);
        XCAFDoc_Location::Set(*lab, t);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

} // extern "C"
