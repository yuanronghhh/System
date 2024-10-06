#!/usr/bin/env python3
import logging
import os
from os import path
from pathlib import Path

logging.basicConfig(format="%(message)s", level=logging.DEBUG)

def path_suffix(pname):
    return path.splitext(pname)[-1]

def parse_object_define(line):
    if not line.startswith("SYS_DEFINE_TYPE_WITH_PRIVATE"):
        return

    offset = -3
    if not line.endswith(";\n"):
        offset = -2

    dinfo = line[29: offset].split(",")
    for i, item in enumerate(dinfo):
        dinfo[i] = item.strip()

    return dinfo

def rename_surffix(fname, nname):
    logging.info("%s\t%s" % (fname, nname))
    os.rename(fname, nname)

def update_camecase_patch(s):
    if not s:
        return

    nstr = ""
    for i in range(0, len(s)):
        if s[i] == '_':
            continue

        if (i > 0 and s[i-1] == '_') or (i == 0):
            nstr += s[i].upper()
        else:
            nstr += s[i]

    return nstr

def to_camecase_name(type_name):
    if not type_name:
        return None

    return update_camecase_patch(type_name)

def insert_patch(s, ps, ch):
    if not s:
        return

    if not ps:
        return

    nstr = ""

    for i in range(0, len(s)):
        nstr += s[i].lower()

        if i in ps:
            nstr += ch

    return nstr

def to_under_line_name(TypeName):
    uidx = []

    if not TypeName:
        return None

    if not TypeName[0].isupper():
        return None

    for i in range(0, len(TypeName)):
        if TypeName[i].isupper():
            uidx.append(i-1)

    return insert_patch(TypeName, uidx, '_')

def rename_data(fname):
    p1 = fname.name.replace("Fr", "FR").replace(".h", "")
    p2 = fname.name.replace(".h", "")
    # print(".replace(\"%s\",\"%s\") \\" % (p1, p2))
    # return

    f = open(fname, "r+", encoding="utf-8")
    data = f.read()
    data = data\
            .replace("FRPServer", "FrpServer") \
            .replace("FRRegion", "FrRegion") \
            .replace("FRSurface", "FrSurface") \
            .replace("FRContext", "FrContext") \
            .replace("FRSInt", "FrSInt") \
            .replace("FRColor", "FrColor") \
            .replace("FRRect", "FrRect") \
            .replace("FRGetBoundFunc", "FrGetBoundFunc") \
            .replace("FRCommon","FrCommon") \
            .replace("FRTypes","FrTypes") \
            .replace("FRCore","FrCore") \
            .replace("FRNode","FrNode") \
            .replace("FRApplication","FrApplication") \
            .replace("FRSource","FrSource") \
            .replace("FRMain","FrMain") \
            .replace("FRWorker","FrWorker") \
            .replace("FREnv","FrEnv") \
            .replace("FRPair","FrPair") \
            .replace("FRFunc","FrFunc") \
            .replace("FRPQueue","FrPQueue") \
            .replace("FRDraw","FrDraw") \
            .replace("FRGraph","FrGraph") \
            .replace("FRCanvasPrivate","FrCanvasPrivate") \
            .replace("FRCanvas","FrCanvas") \
            .replace("FREventCore","FrEventCore") \
            .replace("FREvents","FrEvents") \
            .replace("FRAWatchBuilder","FrAWatchBuilder") \
            .replace("FRAction","FrAction") \
            .replace("FRAWatchCursorMove","FrAWatchCursorMove") \
            .replace("FRAMouseKey","FrAMouseKey") \
            .replace("FRAWatchAny","FrAWatchAny") \
            .replace("FRAWatchMousePress","FrAWatchMousePress") \
            .replace("FRADoubleClick","FrADoubleClick") \
            .replace("FRAWatchKey","FrAWatchKey") \
            .replace("FRAWatch","FrAWatch") \
            .replace("FRAWatchKeyPress","FrAWatchKeyPress") \
            .replace("FRAWatchRefresh","FrAWatchRefresh") \
            .replace("FRACursorMove","FrACursorMove") \
            .replace("FRAKey","FrAKey") \
            .replace("FRAWatchMouseRelease","FrAWatchMouseRelease") \
            .replace("FREventMapping","FrEventMapping") \
            .replace("FREvent","FrEvent") \
            .replace("FREventScroll","FrEventScroll") \
            .replace("FREventNodeKey","FrEventNodeKey") \
            .replace("FREventRefresh","FrEventRefresh") \
            .replace("FREventAny","FrEventAny") \
            .replace("FREventCursorMove","FrEventCursorMove") \
            .replace("FREventMouseKey","FrEventMouseKey") \
            .replace("FREventKey","FrEventKey") \
            .replace("FREventPrivate","FrEventPrivate") \
            .replace("FREventPressed","FrEventPressed") \
            .replace("FRExpr","FrExpr") \
            .replace("FRFont","FrFont") \
            .replace("FRVkvg","FrVkvg") \
            .replace("FRMono","FrMono") \
            .replace("FRGlfw","FrGlfw") \
            .replace("FRCairo","FrCairo") \
            .replace("FRGlib","FrGlib") \
            .replace("FRWindow","FrWindow") \
            .replace("FRDisplay","FrDisplay")

    f.seek(0)
    f.truncate()
    f.write(data)
    f.close()

def update_object_data(f):
    dinfo = []
    lines = f.readlines()
    update = False

    for lnum, line in enumerate(lines):
        if not line:
            break

        if not dinfo:
            dinfo = parse_object_define(line)
            continue
            # if dinfo:
            #     lines[lnum-1] = "SYS_DEFINE_TYPE(%s, %s, %s);\n" % (dinfo[1], dinfo[2], dinfo[0])
            #     break
            # continue

        if dinfo:
            pass
            # if line.find("init(SysObject *o)") > -1:
            #     lines[lnum] = line.replace("SysObject *o", dinfo[0] + " *self")
            #     update = True

            # if line.find(dinfo[1] + "_init(" + dinfo[0] + " *self)") > -1:
            #   if lines[lnum + 1].find("priv") == -1:
            #       if lines[lnum + 1].find("}") == -1:
            #           lines[lnum + 1] = "  %s%s" % (dinfo[0] + "Private *priv = self->priv = " + dinfo[1] + "_get_private(self);\n", lines[lnum + 1])
            #       else:
            #           lines[lnum + 1] = "  %s%s" % ("self->priv = " + dinfo[1] + "_get_private(self);\n", lines[lnum + 1])

            #       continue

    if dinfo and update:
        logging.info(dinfo)
        data = "".join(lines)
        f.seek(0)
        f.truncate()
        f.write(data)
        f.close()

def glfw_to_iface():
    func_data = """\
void (*create) (FrWindow* window, SysInt width, SysInt height, const SysChar *title, FrWindow *share);
FrMonitor* (*get_primary_monitor) ();
void (*set_error_callback) (FrWindowErrFunc callback);
SysPointer (*get_user_pointer) (FrWindow *window);
void (*set_user_pointer) (FrWindow *window, SysPointer user_data);
void (*get_framebuffer_size) (FrWindow *window, SysInt *width, SysInt *height);
void (*get_window_size) (FrWindow *window, SysInt *width, SysInt *height);
void (*set_window_size) (FrWindow *window, SysInt width, SysInt height);
void (*set_window_title) (FrWindow *window, const SysChar *title);
void (*set_window_opacity) (FrWindow *window, SysDouble opacity);
void (*set_window_should_close) (FrWindow *window, SysBool bvalue);
void (*set_window_focus_callback) (FrWindow *window, SysFunc func);
void (*set_window_pos_callback) (FrWindow *window, SysFunc func);
void (*set_window_refresh_callback) (FrWindow *window, SysFunc func);
void (*set_framebuffer_size_callback) (FrWindow *window, SysFunc func);
void (*set_window_size_callback) (FrWindow *window, SysFunc func);
void (*set_key_callback) (FrWindow *window, SysFunc func);
void (*set_mouse_button_callback) (FrWindow *window, SysFunc func);
void (*set_cursor_pos_callback) (FrWindow *window, SysFunc func);
void (*set_window_close_callback) (FrWindow *window, SysFunc func);
void (*set_scroll_callback) (FrWindow *window, SysFunc func);
void (*set_cursor_enter_callback) (FrWindow *window, SysFunc func);
void (*set_window_maximize_callback) (FrWindow *window, SysFunc func);
void (*swap_buffers) (FrWindow *window);
void (*destroy_window) (FrWindow *window);
const SysChar* (*get_key_name) (SysInt key, SysInt scancode);
SysInt (*get_key) (FrWindow *window, SysInt key);
void (*wait_events_timeout) (SysDouble sec);
void (*wait_events) (void);
void (*post_empty_event) (void);
void (*poll_events) (void);
void (*terminate) (void);
void (*window_hit) (SysInt p, SysInt v);
"""
    for line in func_data.split("\n"):
        if not line:
            continue

        idx1 = line.find('(')
        idx2 = line.find(")")

        line = line[idx1+2:idx2]
        nname = line
        print("iface->%s = fr_glfw_%s;" % (nname, nname))

def rename_gobject():
    wk = os.getcwd() + "/Cst"
    wk = "/home/greyhound/Git/Cst"

    for dp, dns, fns in os.walk(wk):
        for f in fns:
            if path_suffix(f) not in {".c", ".h" }:
                continue

            fname = Path(path.join(dp, f))
            # if not fname.name.startswith("App"):
            #     continue

            nname = "%s/%s" % (fname.parent.as_posix(), fname.name.replace("FR", "Fr"))
            rename_data(fname)

if __name__ == '__main__':
    glfw_to_iface()
