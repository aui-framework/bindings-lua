#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import re
import sys

source = sys.argv[1]
destination = sys.argv[2]

RE_TEMPLATE = re.compile(r'^\s*(template<.+>)')
RE_CLASS_DEF = re.compile(r'^\s*(class|struct) (\w+)')
RE_METHOD_DEF = re.compile(r'^\s*(\S+) (\S+)\((.+ [a-zA-Z0-9]+)*\)( const)? override;')
RE_BLOCK_END = re.compile(r'^\s*}\s*;')


def parse_argument(argument):
    RE_ARGUMENT = re.compile(r'(.*) ([a-zA-Z0-9]+)')
    match = RE_ARGUMENT.match(argument)
    return (match.groups()[0], match.groups()[1])

class Parser:
    remaining = ""

    def run(self, source, destination):
        with open(source, 'r') as input:
            with open(destination, 'w') as output:
                self.output = output
                output.write('// This file is generated with lua_reflection.py. Do not modify.\n')

                self.templateDescription = None

                def try_catch_wrapper(to_call):
                    log_tag = 'AClass<std::decay_t<decltype(*this)>>::name()'
                    output.write("      try {\n")
                    output.write(to_call)
                    output.write("      }\n")
                    output.write("      catch(const AException& e) {\n")
                    output.write(f'         ALogger::err({log_tag}) << "Exception occurred after lua function call: " << e;\n')
                    output.write('      }\n')
                    output.write("      catch(const clg::lua_exception& e) {\n")
                    output.write(f'         ALogger::warn({log_tag}) << "Exception occurred after lua function call: " << e.what();\n')
                    output.write('      }\n')
                    output.write("      catch(const std::exception& e) {\n")
                    output.write(f'         ALogger::err({log_tag}) << "Exception occurred after lua function call: " << e.what();\n')
                    output.write('      }\n')
                    output.write("      catch(...) {\n")
                    output.write(f'         ALogger::err({log_tag}) << "Exception occurred after lua function call: unknown exception";\n')
                    output.write('      }\n')

                def visit_template(match):
                    self.templateDescription = match.groups()[0]
                    self.output.write(self.templateDescription)
                    self.output.write("\n")

                    self.process({
                        RE_CLASS_DEF: visit_class_def,
                    })
                def visit_class_def(match):
                    className = match.groups()[1]
                    e = self.remaining.find("{")+1
                    self.output.write(f"class {className} {self.remaining[:e+1]}")
                    self.remaining = self.remaining[e:]

                    self.blockEnd = False
                    self.methods = []

                    def visit_block_end(match):
                        output.write("\nprivate:\n")

                        for method in self.methods:
                            output.write(f"bool m_{method}Flag = false;\n")

                        output.write("static const std::map<std::string_view, bool(LuaExposedView::*)>& ")
                        output.write(f"vtable()")
                        output.write(" {\n")
                        output.write("    static const std::map<std::string_view, bool(LuaExposedView::*)> v = {\n")
                        for method in self.methods:
                            output.write("        { ")
                            output.write("\"")
                            output.write(method)
                            output.write("\", ")
                            output.write(f"&LuaExposedView::m_{method}Flag")
                            output.write(" },\n")

                        output.write("    };\n")
                        output.write("    return v;\n")
                        output.write("}\n")
                        self.output.write("};\n")
                        self.blockEnd = True
                    def visit_method_def(match):
                        returnType = match.groups()[0]
                        isVoid = returnType == 'void'
                        name = match.groups()[1]
                        argsRaw = match.groups()[2]
                        if argsRaw is not None:
                            args = [i.strip() for i in match.groups()[2].split(",")]
                        else:
                            args = {}

                        argNames = ', '.join([parse_argument(i)[1] for i in args])

                        output.write('\n')
                        output.write(returnType)
                        output.write(' ')
                        output.write(name)
                        output.write('(')
                        output.write(", ".join(args))
                        output.write(') override {\n')

                        if name == "render":
                            output.write("performance::AUI_VIEW_RENDER += 1;\n")


                        def createSuperCall():
                            output.write(f'View::{name}')

                            output.write('(')
                            output.write(argNames)
                            output.write(');\n')

                        def createSuperCallWithResult():
                            output.write(f'auto superCallResult = View::{name}')

                            output.write('(')
                            output.write(argNames)
                            output.write(');\n')

                        output.write('  ')
                        if isVoid:
                            createSuperCall()
                            output.write(f'     if (!m_{name}Flag) return;\n')
                        else:
                            createSuperCallWithResult()
                            output.write(f'     if (!m_{name}Flag) return superCallResult;\n')

                        output.write(f'     if (auto func = luaDataHolder()["{name}"].template is<clg::function>())\n')
                        output.write('      {\n')

                        if not argNames:
                            argsNamesWithComma = ""
                        else:
                            argsNamesWithComma = f', {argNames}'

                        if isVoid:
                            try_catch_wrapper(f'            (*func)(aui::ptr::shared_from_this(this){argsNamesWithComma});\n')
                        else:
                            try_catch_wrapper(f'            return (*func).template call<{returnType}>(aui::ptr::shared_from_this(this){argsNamesWithComma});\n')
                        output.write('    }\n')

                        if not isVoid:
                            output.write(f'    return superCallResult;\n')

                        self.methods.append(name)
                        output.write('}\n')

                    while not self.blockEnd:
                        self.process({
                            RE_METHOD_DEF: visit_method_def,
                            RE_BLOCK_END: visit_block_end
                        })

                self.remaining = "\n".join(input.readlines())

                while len(self.remaining) > 0:
                    self.process({
                        RE_TEMPLATE: visit_template,
                        RE_CLASS_DEF: visit_class_def,
                    })


    def process(self, table):
        if len(self.remaining) == 0:
            raise EOFError()
        for pattern, callback in table.items():
            match = pattern.match(self.remaining)
            if match is not None:
                self.remaining = self.remaining[match.end():]
                callback(match)
                return
        self.output.write(self.remaining[:self.remaining.find("\n") + 1])
        self.remaining = self.remaining[self.remaining.find("\n") + 2:]


p = Parser()
p.run(source, destination)
