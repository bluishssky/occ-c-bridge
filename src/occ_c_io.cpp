// Copyright (c) 2026 Yonghua Liao
// Author: Yonghua Liao
// SPDX-License-Identifier: Apache-2.0
#include "occ_c_bridge.h"
#include "handle_table.h"

// OpenCASCADE（只允许出现在中间层内部）
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <Interface_Static.hxx>
#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
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

using StepReaderHandle = std::shared_ptr<STEPCAFControl_Reader>;
using StepWriterHandle = std::shared_ptr<STEPCAFControl_Writer>;
using DocHandle = Handle(TDocStd_Document);

HandleTable& table() {
    return HandleTable::instance();
}

TCollection_ExtendedString toExtString(const char* path) {
    return TCollection_ExtendedString(path, Standard_True);
}

} // namespace

extern "C" {

OcbStatus OCB_CALL ocb_step_read(const char* path, OcbShape* out) {
    if (!path || !out) {
        setError(OCB_ERR_INVALID_ARG, "path or out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        STEPControl_Reader reader;
        const IFSelect_ReturnStatus status = reader.ReadFile(path);
        if (status != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP read failed");
            return OCB_ERR_IO;
        }
        if (reader.TransferRoots() == 0) {
            setError(OCB_ERR_IO, "STEP transfer produced no shapes");
            return OCB_ERR_IO;
        }
        TopoDS_Shape shape = reader.OneShape();
        if (shape.IsNull()) {
            setError(OCB_ERR_IO, "STEP produced null shape");
            return OCB_ERR_IO;
        }
        *out = table().add(std::move(shape));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_step_write(OcbShape shape, const char* path) {
    TopoDS_Shape* s = table().get<TopoDS_Shape>(shape);
    if (!path || !s || s->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid shape handle or path");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        STEPControl_Writer writer;
        const IFSelect_ReturnStatus status = writer.Transfer(*s, STEPControl_AsIs);
        if (status != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP transfer failed");
            return OCB_ERR_IO;
        }
        const IFSelect_ReturnStatus writeStatus = writer.Write(path);
        if (writeStatus != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP write failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepreader_create(OcbStepReader* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        auto reader = std::make_shared<STEPCAFControl_Reader>();
        reader->SetColorMode(true);
        reader->SetNameMode(true);
        *out = table().add(reader);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepreader_set_color_mode(OcbStepReader r, bool mode) {
    StepReaderHandle* reader = table().get<StepReaderHandle>(r);
    if (!reader || !reader->get()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step reader handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    (*reader)->SetColorMode(mode);
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_stepreader_set_name_mode(OcbStepReader r, bool mode) {
    StepReaderHandle* reader = table().get<StepReaderHandle>(r);
    if (!reader || !reader->get()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step reader handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    (*reader)->SetNameMode(mode);
    return OCB_OK;
}

/* 等价 Interface_Static::SetIVal(name, value)：STEP 读取参数（全局）。
   name 可为空表示忽略（仅设置 value，对 "read.*" 参数无意义，保留以对齐签名）。 */
OcbStatus OCB_CALL ocb_stepreader_set_param_int(const char* name, int value) {
    if (!name || !*name) {
        setError(OCB_ERR_INVALID_ARG, "name is empty");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Interface_Static::SetIVal(name, value);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepreader_set_param_real(const char* name, double value) {
    if (!name || !*name) {
        setError(OCB_ERR_INVALID_ARG, "name is empty");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        Interface_Static::SetRVal(name, value);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepreader_read_file(OcbStepReader r, const char* path) {
    StepReaderHandle* reader = table().get<StepReaderHandle>(r);
    if (!reader || !reader->get() || !path) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step reader handle or path");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        const IFSelect_ReturnStatus status = (*reader)->ReadFile(path);
        if (status != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP read failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepreader_transfer(OcbStepReader r, OcbDoc doc) {
    StepReaderHandle* reader = table().get<StepReaderHandle>(r);
    DocHandle* d = table().get<DocHandle>(doc);
    if (!reader || !reader->get() || !d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step reader/doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if (!(*reader)->Transfer(*d)) {
            setError(OCB_ERR_IO, "STEP transfer failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_stepreader_release(OcbStepReader r) {
    table().release(r);
}

OcbStatus OCB_CALL ocb_stepwriter_create(OcbStepWriter* out) {
    if (!out) {
        setError(OCB_ERR_INVALID_ARG, "out is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    try {
        *out = table().add(StepWriterHandle(new STEPCAFControl_Writer()));
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_transfer(OcbStepWriter w, OcbDoc doc) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    DocHandle* d = table().get<DocHandle>(doc);
    if (!writer || !d || !(*d)) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer/doc handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if ((*writer)->Transfer(*d) != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP transfer failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_write(OcbStepWriter w, const char* path) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    if (!writer || !path) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer handle or path");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        if ((*writer)->Write(path) != IFSelect_RetDone) {
            setError(OCB_ERR_IO, "STEP write failed");
            return OCB_ERR_IO;
        }
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_set_color_mode(OcbStepWriter w, bool mode) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    if (!writer) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*writer)->SetColorMode(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_set_name_mode(OcbStepWriter w, bool mode) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    if (!writer) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*writer)->SetNameMode(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_set_layer_mode(OcbStepWriter w, bool mode) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    if (!writer) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*writer)->SetLayerMode(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

OcbStatus OCB_CALL ocb_stepwriter_set_material_mode(OcbStepWriter w, bool mode) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    if (!writer) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        (*writer)->SetMaterialMode(mode);
        return OCB_OK;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

/* 只迁移某个 OCAF 子树(与业务“按 projectLabel 导出”语义一致,区别于整文档迁移) */
OcbStatus OCB_CALL ocb_stepwriter_transfer_label(OcbStepWriter w, OcbLabel label) {
    StepWriterHandle* writer = table().get<StepWriterHandle>(w);
    TDF_Label* lab = table().get<TDF_Label>(label);
    if (!writer || !lab || lab->IsNull()) {
        setError(OCB_ERR_INVALID_HANDLE, "invalid step writer/label handle");
        return OCB_ERR_INVALID_HANDLE;
    }
    clearError();
    try {
        return (*writer)->Transfer(*lab) ? OCB_OK : OCB_ERR_IO;
    }
    OCB_CATCH_RETURN(OCB_ERR_OCC_EXCEPTION)
}

void OCB_CALL ocb_stepwriter_release(OcbStepWriter w) {
    table().release(w);
}

OcbStatus OCB_CALL ocb_interface_set_int(const char* key, int value) {
    if (!key) {
        setError(OCB_ERR_INVALID_ARG, "key is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    if (!Interface_Static::SetIVal(key, value)) {
        setError(OCB_ERR_INVALID_ARG, "unknown or read-only parameter");
        return OCB_ERR_INVALID_ARG;
    }
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_interface_set_real(const char* key, double value) {
    if (!key) {
        setError(OCB_ERR_INVALID_ARG, "key is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    if (!Interface_Static::SetRVal(key, value)) {
        setError(OCB_ERR_INVALID_ARG, "unknown or read-only parameter");
        return OCB_ERR_INVALID_ARG;
    }
    return OCB_OK;
}

OcbStatus OCB_CALL ocb_interface_set_string(const char* key, const char* value) {
    if (!key || !value) {
        setError(OCB_ERR_INVALID_ARG, "key or value is null");
        return OCB_ERR_INVALID_ARG;
    }
    clearError();
    if (!Interface_Static::SetCVal(key, value)) {
        setError(OCB_ERR_INVALID_ARG, "unknown or read-only parameter");
        return OCB_ERR_INVALID_ARG;
    }
    return OCB_OK;
}

} // extern "C"
