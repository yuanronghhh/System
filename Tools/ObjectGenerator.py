#!/usr/bin/env python3

import abc
import argparse
import os
import re
import sys
import logging

from AppJsonEncoder import Serializer
from pathlib import Path

logging.basicConfig(format="%(message)s", level=logging.DEBUG)


interface_h_template = """\
#ifndef __${TYPE_NAME}__
#define __${TYPE_NAME}__

#include <Framework/FrCommon.h>

SYS_BEGIN_DECLS

#define ${FN_TYPE_NAME} (${type_name}_get_type())
#define ${TYPE_NAME}(o) ((${TypeName}* )sys_object_cast_check(o, ${FN_TYPE_NAME}))
#define ${TYPE_NAME}_GET_IFACE(o) ((${TypeName}Interface *)SYS_TYPE_GET_INTERFACE(o, ${FN_TYPE_NAME}))

typedef struct _${TypeName} ${TypeName};
typedef struct _${TypeName}Interface ${TypeName}Interface;

${struct_str}

SysType ${type_name}_get_type(void);
${FUNC_DEFINE_CODES}

SYS_END_DECLS

#endif\
"""

interface_c_template = """\
#include <${header_path}/${TypeName}.h>

SYS_DEFINE_INTERFACE(${TypeName}, ${type_name}, SYS_TYPE_OBJECT);

/* object api */
void ${type_name}_default_init(${TypeName}Interface* iface) {
}
${FUNC_IMPL_CODES}\
"""

interface_h_func_return_template = """\
${func_return} ${type_name}_${func_name} (${func_args});\
"""

interface_h_func_template = """\
void ${type_name}_${func_name} (${func_args});\
"""

interface_func_return_template = """\
${func_return} ${type_name}_${func_name} (${func_args}) {
  sys_return_val_if_fail(self != NULL, ${return_default});

  return ${TYPE_NAME}_GET_IFACE(self)->${func_name}(${func_args_name});
}
"""

interface_func_template = """\
void ${type_name}_${func_name} (${func_args}) {
  sys_return_if_fail(self != NULL);

  ${TYPE_NAME}_GET_IFACE(self)->${func_name}(${func_args_name});
}
"""

h_template = """\
#ifndef __${TYPE_NAME}_H__
#define __${TYPE_NAME}_H__

#include <${common_header}>

SYS_BEGIN_DECLS

#define ${FN_TYPE_NAME} (${type_name}_get_type())
#define ${TYPE_NAME}(o) ((${TypeName}* )sys_object_cast_check(o, ${FN_TYPE_NAME}))
#define ${TYPE_NAME}_CLASS(o) ((${TypeName}Class *)sys_class_cast_check(o, ${FN_TYPE_NAME}))
#define ${TYPE_NAME}_GET_CLASS(o) sys_instance_get_class(o, ${TypeName}Class)

typedef struct _${TypeName}Context ${TypeName}Context;
typedef struct _${TypeName} ${TypeName};
typedef struct _${TypeName}Class ${TypeName}Class;

struct _${TypeName}Class {
  ${ParentTypeName}Class parent;
};
${struct_str}
struct _${TypeName}Context {
};

SYS_API SysType ${type_name}_get_type(void);
SYS_API ${SelfTypeName} *${type_name}_new(void);

SYS_API ${SelfTypeName} *${type_name}_new_I(${TypeName}Context *info);

SYS_END_DECLS

#endif\
"""


c_template = """\
#include <${header_path}/${TypeName}.h>

SYS_DEFINE_TYPE(${TypeName}, ${type_name}, ${TYPE_PARENT});

/* object api */
static void ${type_name}_construct_i(${SelfTypeName} *self, ${TypeName}Context *info) {
}

${SelfTypeName}* ${type_name}_new(void) {
  return sys_object_new(${FN_TYPE_NAME}, NULL);
}

${SelfTypeName} *${type_name}_new_I(${TypeName}Context *info) {
  ${SelfTypeName} *o = ${type_name}_new();

  ${type_name}_construct_i(o, info);

  return o;
}

static void ${type_name}_dispose(SysObject* o) {
  ${TypeName} *self = ${TYPE_NAME}(o);

  UNUSED(self);

  SYS_OBJECT_CLASS(${type_name}_parent_class)->dispose(o);
}

static void ${type_name}_class_init(${TypeName}Class* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = ${type_name}_dispose;
}

static void ${type_name}_init(${TypeName}* self) {
}
"""

default_return_dict = {
        "SysDouble": "-1",
        "SysInt": "-1",
        "*": "NULL",
        "void": "void"
        }

func_proto_re = re.compile(r"\(\*([a-zA-Z0-9_]+)\)\s*\(([^)]*)\)\s*;")
func_define_re = re.compile(r"([a-zA-Z0-9_]+)\(([^)]*)\);")
prop_re = re.compile(r"([a-zA-Z0-9_]+)\s*;")
type_re = re.compile(r"^\s*([a-zA-Z0-9_]+\s*\*{0,1})")


def match_return_default(nstr):
    nstr = nstr.strip(" ")

    if nstr.find("*") > -1:
        return default_return_dict["*"]

    if nstr in default_return_dict:
        return default_return_dict[nstr]

    return "void"

def parse_args_list(arg_str):
    if not arg_str:
        return []

    if arg_str == "void":
        return []

    arg_info_list = []
    args = arg_str.split(",")

    for arg in args:
        name = ""
        alen = len(arg)
        if alen == 0:
            continue

        argInfo = Arg()
        for i in range(alen-1, 0, -1):
            c = arg[i]
            if not c.isalnum() and c != "_":
                break

            name += c

        argInfo.name = name[::-1]
        argInfo.type = arg[0:alen-len(name)]

        arg_info_list.append(argInfo)

    return arg_info_list

def parse_prop_value(match_type, line):
    prop = Prop()

    prop.type = match_type
    match = prop_re.findall(line)
    if not match:
        return None
    prop.name = match[0]

    return prop

def parse_prop_type(line):
    match = type_re.findall(line)
    if not match:
        return None

    return match[0].strip(" ");

def parse_func_define(match_type, func_start):
    prop = Prop()
    if not match_type:
        return None

    match = func_define_re.findall(func_start)
    if not match:
        return None

    prop.type = match_type
    prop.name = match[0][0]
    prop.args = match[0][1]
    prop.args_list = parse_args_list(match[0][1])

    return prop

def parse_func_prototype(match_type, line):
    prop = Prop()
    prop.type = match_type
    match = func_proto_re.findall(line)
    if not match:
        return None

    prop.name = match[0][0]
    prop.args = match[0][1]
    prop.args_list = parse_args_list(match[0][1])

    return prop

class Prop:
    def __init__(self):
        self.type = None
        self.name = None
        self.args = None
        self.args_list = []

class Arg:
    def __init__(self):
        self.type = None
        self.name = None

class TemplateInfo:
    struct_name_re = re.compile(r"struct _(\w+)")

    def __init__(self, struct_str):
        self.struct_str = struct_str
        self.tpl = struct_str.strip(" ").split("\n")

        self.struct = self.parse_struct(self.tpl[1])
        self.sep_struct = self.seperate_struct(self.struct)
        self.is_interface = False
        if self.sep_struct[-1] == "Interface":
            self.is_interface = True
            self.sep_struct = self.sep_struct[:-1]

        self.props = self.parse_props(self.tpl[2:-1])

        if not self.props:
            logging.error("struct not parse parent info: %s", self.tpl[2])
            sys.exit(-1)
        else:
            self.p_sep_struct = self.seperate_struct(self.props[0].type)

    def parse_prop(self, line):
        prop = None
        if line.startswith("#"):
            return None

        match_type = parse_prop_type(line)
        if not match_type:
            return None

        if self.is_interface and match_type != "SysObject" and match_type != "SysTypeInterface":
            prop = parse_func_prototype(match_type, line);
        else:
            prop = parse_prop_value(match_type, line)

        if not prop:
            return None

        return prop

    def parse_pinfo(self, first_str):
        prop = self.parse_prop(first_str)
        if not prop:
            return None
        return prop

    def parse_props(self, lines):
        props = []
        for line in lines:
            prop = self.parse_prop(line)
            if not prop:
                continue

            props.append(prop)
        return props

    def parse_struct(self, line):
        match = TemplateInfo.struct_name_re.findall(line)
        if not match:
            return None

        return match[0]


    def parse_struct_name(self, prefix, struct):
        return struct.replace(prefix, "")

    def is_satify(self, b, i, c, nc):
        if not nc:
            return True

        if i <= b:
            return False

        return True

    def seperate_struct(self, struct_name):
        b = 0
        data = []
        slen = len(struct_name)

        for i in range(0, slen):
            c = struct_name[i]

            if i < slen-1:
                if not c.isupper():
                    continue

            nc = None
            if i+1 < slen:
                nc = struct_name[i+1]

            if self.is_satify(b, i, c, nc):
                if nc:
                    data.append(struct_name[b:i])
                else:
                    data.append(struct_name[b:i+1])

                b = i

        return data

    def parse_Prefix(self, struct_name):
        data = self.seperate_struct(struct_name)
        if not data:
            return None

        return data[0]

    def get_TypeName(self):
        return "".join(self.sep_struct)

    def get_typename(self):
        return "".join(self.sep_struct).lower()

    def get_type_name(self):
        return "_".join(self.sep_struct).lower()

    def get_struct_str(self):
        return self.struct_str

    def get_TYPE_NAME(self):
        return "_".join(self.sep_struct).upper()

    def get_PARENT_TYPE(self):
        if not self.p_sep_struct:
            return ""

        return "_".join(self.p_sep_struct).upper()

    def get_ParentTypeName(self):
        if not self.p_sep_struct:
            return ""

        return "".join(self.p_sep_struct)

    def get_FN_TYPE_NAME(self):
        if not self.p_sep_struct:
            return ""

        return ("%s_TYPE_%s" % (self.sep_struct[0], "_".join(self.sep_struct[1:]))).upper()

    def get_TYPE_PARENT(self):
        if not self.p_sep_struct:
            return ""

        return ("%s_TYPE_%s" % (self.p_sep_struct[0], "_".join(self.p_sep_struct[1:]))).upper()

class TemplateGenerator:
    def __init__(self, relpath, header_path, common_header):
        self.relpath = Path(relpath).as_posix()
        self.header_path = header_path
        self.common_header = common_header
        self.dstDir = Path(relpath).absolute().as_posix()

    @staticmethod
    def gen_with_tpl(info, tpl, header_path, common_header):
        parentType = info.get_ParentTypeName()
        selfType = info.get_TypeName()

        if parentType != "SysObject" and \
                parentType != "SysTypeInstance":
            selfType = parentType

        r = tpl.replace("${TYPE_NAME}", info.get_TYPE_NAME())\
                .replace("${FN_TYPE_NAME}", info.get_FN_TYPE_NAME())\
                .replace("${PARENT_TYPE}", info.get_PARENT_TYPE())\
                .replace("${TYPE_PARENT}", info.get_TYPE_PARENT())\
                .replace("${ParentTypeName}", parentType)\
                .replace("${SelfTypeName}", selfType)\
                .replace("${typename}", info.get_typename())\
                .replace("${TypeName}", info.get_TypeName())\
                .replace("${type_name}", info.get_type_name())\
                .replace("${struct_str}", info.get_struct_str())\
                .replace("${common_header}", common_header)\
                .replace("${header_path}", header_path)

        return r


    def generate_file(self, info):
        if info.is_interface:
            self.generate_interface_file(info)
            return

        h_file = self.dstDir + "/" + info.get_TypeName() + ".h"
        c_file = self.dstDir + "/" + info.get_TypeName() + ".c"

        result = self.gen_with_tpl(info,
                                   h_template, 
                                   self.header_path, 
                                   self.common_header)
        fp = open(h_file, "w+")
        fp.write(result)
        fp.close()

        result = self.gen_with_tpl(info,
                                   c_template, 
                                   self.header_path, 
                                   self.common_header)
        fp = open(c_file, "w+")
        fp.write(result)
        fp.close()

    def gen_interface_c_func_codes(self, prop, info):
        tpl = ""

        return_code = prop.type
        if return_code == "SysTypeInterface" or return_code == "SysObject":
            return ""

        return_default = match_return_default(return_code)
        if prop.type == "void":
            tpl = interface_func_template
        else:
            tpl = interface_func_return_template

        func_codes = tpl\
                .replace("${TYPE_NAME}", info.get_TYPE_NAME())\
                .replace("${TypeName}", info.get_TypeName())\
                .replace("${TypeNameNoI}", info.get_TypeName().replace("_i_", "_"))\
                .replace("${type_name}", info.get_type_name())\
                .replace("${type_name_no_i}", info.get_type_name().replace("_i_", "_"))\
                .replace("${func_name}", prop.name)\
                .replace("${func_return}", return_code)\
                .replace("${return_default}", return_default)\
                .replace("${func_args}", prop.args)\
                .replace("${func_args_name}", ", ".join([a.name for a in prop.args_list]))

        return func_codes

    def gen_interface_h_func_codes(self, prop, info):
        tpl = ""
        return_code = prop.type
        if return_code == "SysTypeInterface":
            return ""

        if prop.type == "void":
            tpl = interface_h_func_template
        else:
            has_return = True
            tpl = interface_h_func_return_template

        func_codes = tpl\
                .replace("${TYPE_NAME}", info.get_TYPE_NAME())\
                .replace("${TypeName}", info.get_TypeName())\
                .replace("${type_name}", info.get_type_name())\
                .replace("${func_name}", prop.name)\
                .replace("${func_return}", return_code)\
                .replace("${func_args}", prop.args)

        return func_codes


    def gen_c_result(self, info):
        result = self.gen_with_tpl(info, interface_c_template, self.header_path, self.common_header)

        func_codes = ""
        for prop in info.props:
            func_codes += self.gen_interface_c_func_codes(prop, info)
            func_codes += "\n"

        func_codes = func_codes[0:-1]

        result = result.replace("${FUNC_IMPL_CODES}", func_codes)
        return result

    def gen_c_interface_result(self, info):
        result = self.gen_with_tpl(info, interface_c_template, self.header_path, self.common_header)

        func_codes = ""
        for prop in info.props:
            func_codes += self.gen_interface_c_func_codes(prop, info)
            func_codes += "\n"

        func_codes = func_codes[0:-1]

        result = result.replace("${FUNC_IMPL_CODES}", func_codes)
        return result

    def gen_h_interface_result(self, info):
        result = self.gen_with_tpl(info, interface_h_template, self.header_path, self.common_header)
        has_return = False

        func_codes = ""
        for prop in info.props:
            func_codes += self.gen_interface_h_func_codes(prop, info)
            func_codes += "\n"

        result = result.replace("${FUNC_DEFINE_CODES}", func_codes)
        return result

    def generate_interface_file(self, info):
        h_file = self.dstDir + "/" + info.get_TypeName() + ".h"
        c_file = self.dstDir + "/" + info.get_TypeName() + ".c"

        result = self.gen_h_interface_result(info)
        fp = open(h_file, "w+")
        fp.write(result)
        fp.close()

        result = self.gen_c_interface_result(info)
        fp = open(c_file, "w+")
        fp.write(result)
        fp.close()

    def match_type(self, type_str):
        internal_type = {
                         "SysInt": "SYS_TYPE_INT",
                         "SysDouble": "SYS_TYPE_DOUBLE",
                         "SysChar *": "SYS_TYPE_STRING",
                         "SysChar*": "SYS_TYPE_STRING",
                         "SysObject *": "SYS_TYPE_OBJECT",
                         "SysBool": "SYS_TYPE_BOOL",
                         "SysFloat": "SYS_TYPE_FLOAT",
                         "SysPointer": "SYS_TYPE_POINTER"
                         }

        if type_str.endswith("Func"):
            return "SYS_TYPE_POINTER"

        if type_str in internal_type:
            return internal_type[type_str]

        if type_str.endswith("*"):
            return "SYS_TYPE_OBJECT?"

        return -1

    def generate_field(self, info):
        props = info.props

        #define sys_object_add_property(TYPE, TypeName, full_type, field_type, field_name) \
        tpl = "sys_object_add_property(%s, %s, \"%s\", %s, %s);"

        for p in props:
            if p.name == "parent" or p.name == "unowned":
                continue

            data_type = self.match_type(p.type)

            print(tpl % (
                info.get_FN_TYPE_NAME(),
                info.get_TypeName(),
                p.type,
                data_type,
                p.name))
