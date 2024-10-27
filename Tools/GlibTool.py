#!/usr/bin/env python3
import logging
import os
from os import path
from pathlib import Path

logging.basicConfig(format="%(message)s", level=logging.DEBUG)


def rename_tbox(file):
    f = open(file, "r+", encoding="utf-8")
    data = f.read()
    ndata = data\
            .replace("SysBool", "tb_bool_t")\
            .replace("NULL", "tb_null_t")\
            .replace("sys_debug_N", "tb_trace_d")\
            .replace("sys_error_N", "tb_trace_e")\
            .replace("sys_info_N", "tb_trace_i")\
            .replace("tb_return_if_fail", "tb_assert_and_check_return")\
            .replace("SYS_TYPE_OBJECT", "TB_TYPE_TOBJECT")\
            .replace("IFaceEntry", "tb_iface_entry_t")\
            .replace("IFaceData", "tb_iface_data_t")\
            .replace("TypeData", "tb_type_data_t")\
            .replace("SysTypeInitFunc", "tb_type_init_func_t")\
            .replace("SysInstanceInitFunc", "tb_instance_init_func_t")\
            .replace("SysTypeFinalizeFunc", "tb_finalize_func_t")\
            .replace("SysRecMutex", "tb_rec_mutex_t")\
            .replace("SysParamClass", "tb_param_class_t")\
            .replace("SysParam", "tb_param_t")\
            .replace("SysHArray", "tb_harray_t")\
            .replace("SysSList", "tb_slist_t")\
            .replace("SysHashTable", "tb_hash_table_t")\
            .replace("SysMutex", "tb_mutex_t")\
            .replace("SysUInt", "tb_uint_t")\
            .replace("SysRWLock", "tb_rw_lock_t")\
            .replace("SysDestroyFunc", "tb_destroy_func_t")\
            .replace("SysPointer", "tb_pointer_t")\
            .replace("SysTypeInfo", "tb_type_info_t")\
            .replace("SysRefHook", "tb_ref_hook_func_t")\
            .replace("SysTypeInterface", "tb_type_interface_t")\
            .replace("SysTypeClass", "tb_type_class_t")\
            .replace("TypeNode", "tb_type_node_t")\
            .replace("SysTypeClass", "tb_type_class_t")\
            .replace("SysObjectClass", "tb_tobject_class_t")\
            .replace("SysObject", "tb_tobject_t")\
            .replace("sys_return_if_fail", "tb_assert_and_check_return")\
            .replace("sys_return_val_if_fail", "tb_assert_and_check_return_val")\
            .replace("SYS_REF_CHECK", "TB_REF_CHECK")\
            .replace("sys_type_class", "tb_type_class")\
            .replace("InstanceData", "tb_instance_data_t")\
            .replace("SysRef", "tb_ref_t")\
            .replace("SysType", "tb_type_t")\
            .replace("SYS_", "TB_")\
            .replace("sys_", "tb_")\
            .replace("SysChar", "tb_char_t")\
            .replace("SysInt", "tb_int_t")

    f.seek(0)
    f.truncate()
    f.write(ndata)
    f.close()

def rename_glib(file):
    f = open(file, "r+", encoding="utf-8")
    data = f.read()
    ndata = data\
            .replace("gboolean", "SysBool")\
            .replace("GReal", "SysReal")\
            .replace("GQueue", "SysQueue")\
            .replace("GAsyncQueue", "SysAsyncQueue")\
            .replace("gpointer", "SysPointer")\
            .replace("GError", "SysError")\
            .replace("GThread", "SysThread")\
            .replace("gsize", "SysSize")\
            .replace("GPrivate", "SysPrivate")\
            .replace("gushort", "SysUShort")\
            .replace("guint", "SysUInt")\
            .replace("gchar", "SysChar")\
            .replace("gulong", "SysULong")\
            .replace("TRUE", "true")\
            .replace("FALSE", "false")\
            .replace("GMutex", "SysMutex")\
            .replace("GCond", "SysCond")\
            .replace("GSList", "SysSList")\
            .replace("GRWLock", "SysRWLock")\
            .replace("GRecMutex", "SysRecMutex")\
            .replace("compare_and_exchange", "cmpxchg")\
            .replace("g_strerror", "sys_strerr")\
            .replace("sys_set_error", "sys_error_set_N")\
            .replace("sys_strerror", "sys_strerr")\
            .replace("sys_abort", "sys_abort_N")\
            .replace("sys_warning", "sys_warning_N")\
            .replace("sys_critical", "sys_abort_N")\
            .replace("GPtrArray", "SysPtrArray")\
            .replace("GDestroyNotify", "SysDestroyFunc")\
            .replace("GHashTable", "SysHashTable")\
            .replace("GIdleSource", "SysIdleSource")\
            .replace("GTimeoutSource", "SysTimeoutSource")\
            .replace("GPoll", "SysPoll")\
            .replace("GSource", "SysSource")\
            .replace("GChildWatchSource", "SysChildWatchSource")\
            .replace("GMain", "SysMain")\
            .replace("GWin32PollThreadData", "SysWin32PollThreadData")\
            .replace("GList", "SysList")\
            .replace("GPollFD", "SysPollFD")\
            .replace("GWakeup", "SysWakeup")\
            .replace("GCompareDataFunc", "SysCompareDataFunc")\
            .replace("GLIB_AVAILABLE_IN_2_58\n", "")\
            .replace("GLIB_AVAILABLE_IN_2_68\n", "")\
            .replace("GLIB_AVAILABLE_IN_2_46\n", "")\
            .replace("GLIB_AVAILABLE_STATIC_INLINE_IN_2_70\n", "")\
            .replace("GLIB_AVAILABLE_IN_2_74\n", "")\
            .replace("GLIB_AVAILABLE_IN_ALL\n", "")\
            .replace("GLIB_.*", "")\
            .replace("GLIB_VAR", "")\
            .replace("GIOCondition", "SysIOCondition")\
            .replace("GUnixSignalWatchSource", "SysUnixSignalWatchSource")\
            .replace("G_", "SYS_")\
            .replace("g_", "sys_")\
            .replace("gint", "SysInt")\
            .replace("guchar", "SysUChar")\
            .replace("GINT", "SYSINT")\
            .replace("GArray", "SysArray")\
            .replace("GTimeZone", "SysTimeZone")\
            .replace("GTimeType", "SysTimeType")

    f.seek(0)
    f.truncate()
    f.write(ndata)
    f.close()

if __name__ == '__main__':
    rename_glib(Path("System/Platform/Unix/SysFileUnix.c").as_posix())
    # rename_glib("/home/greyhound/Git/CstDemo/Cst/System/DataTypes/SysTimeZone.h")
