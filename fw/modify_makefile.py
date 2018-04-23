#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
from pathlib import Path

CMD_FLASH = \
    """
#######################################
# flash
#######################################
flash:
\t@{} -c port=SWD reset=HWrst -w build/$(TARGET).bin 0x08000000 -v -rst
"""

CMD_BUILD_AND_FLASH = \
    """
build_and_flash: all
\t@{} -c port=SWD reset=HWrst -w build/$(TARGET).bin 0x08000000 -v -rst
"""

CMD_OBJECTS_APPEND_CPP = \
    """
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
"""

CMD_BUILD_CPP = \
    """
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
\t$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
"""

CFLAGS = (
    '$(MCU)',
    '-std=c++11',
    '-Wno-write-strings',
    '-specs=nano.specs',
    '-specs=nosys.specs',
    '$(C_DEFS)',
    '$(C_INCLUDES)',
    '$(OPT)',
    '-Wall',
    '-fdata-sections',
    '-ffunction-sections',
)

LDFLAGS = (
    '$(MCU)',
    '-Wl,--no-wchar-size-warning',
    '-specs=nosys.specs',
    '-specs=nano.specs',
    '-T$(LDSCRIPT)',
    '$(LIBDIR)',
    '$(LIBS)',
)

TAG_GENERIC = '# Generic Makefile (based on gcc)'
TAG_MODIFY_CPP = '# Modified for C++'
TAG_SOURCES_PATH = '# source path'
TAG_DEFINES_ASM = '# AS defines' \
TAG_DEFINES_C = '# C defines' \
TAG_EOF = '# *** EOF ***' \
TAG_INCLUDES_ASM = '# AS includes' \
TAG_INCLUDES_C = '# C includes' \
TAG_LIFT_OF_ASM_OBJECTS = '# list of ASM program objects' \
TAG_LIST_OF_CPP_OBJECTS = '# list of C++ objects' \
TAG_LIST_OF_OBJECTS = '# list of objects' \
TAG_SOURCES_ASM = '# ASM sources' \
TAG_SOURCES_C = '# C sources' \
TAG_SOURCES_CPP = '# C++ sources'









class NotExist(Exception):
    def __init__(self, name: str):
        self.error_text = '{} is not defined!'.format(name)

    def __str__(self):
        return self.error_text


class MakefileIsModified(Exception):
    def __str__(self):
        return 'This Makefile was already mofified!'


class ProgrammerNotFound(Exception):
    def __str__(self):
        return 'Install STM32 Programmer CLI for Flash firmware.'


class Makefile(object):
    def __init__(self, path: str='Makefile'):
        self.MAKEFILE_LOCATION = path
        with open(self.MAKEFILE_LOCATION, 'r+') as fr:
            self.makefile = fr.read()
        self.modify()

    def __del__(self):
        with open(self.MAKEFILE_LOCATION, 'w') as fw:
            fw.write(self.makefile)

    def update(self, *content: str):
        self.makefile = ''.join(content)

    def replace(self, old: str, new: str):
        self.update(self.makefile.replace(old, new))

    def unix_end_line(self):
        self.replace('\r\n', '\n')

    def get_position_front(self, expression: str) -> int:
        return self.makefile.find(expression) - 1

    def get_position(self, expression: str) -> int:
        return self.makefile.find(expression)

    def get_position_behind(self, expression: str) -> int:
        return self.makefile.find(expression) + len(expression) + 1

    def flags(self, list_of_flags: tuple) -> str:
        return ''.join(map(lambda x: x + ' ', list_of_flags))[:-1]

    def check_existence(self, name: str):
        if self.get_position(name) == -1:
            raise NotExist(name)

    def set_variable(self, name: str, value: str):
        self.check_existence(name)
        position_start = self.get_position_behind(name + ' ')
        i = position_start
        while self.makefile[i] != '\n':
            i += 1
        position_end = i
        self.update(self.makefile[:position_start], ' ', value, self.makefile[position_end:])

    def check_was_modified(self):
        if self.get_position(TAG_MODIFY_CPP) != -1:
            raise MakefileIsModified
        else:
            position = self.get_position_behind(TAG_GENERIC)
            self.update(self.makefile[:position], TAG_MODIFY_CPP, '\n', self.makefile[position:])

    def block_get_position(self, tag: str) -> tuple:
        self.check_existence(tag)
        position_start = self.get_position_behind(tag)
        i = position_start
        while self.makefile[i:i + 2] != '\n\n' and self.makefile[i:i + 2] != '\n#':
            i += 1
        position_end = i
        return (position_start, position_end)

    def block_get(self, tag: str) -> list:
        position_start, position_end = self.block_get_position(tag)
        return self.makefile[position_start:position_end].split('\n')

    def block_set(self, tag: str, content: list):
        position_start, position_end = self.block_get_position(tag)
        code = self.block_get(tag)
        var = code[0] + '\n'
        if content:
            code = sorted(
                list(set(map(
                    lambda x: x.replace('\\', '').strip(),
                    content,
                )))
            )
            code = ''.join(list(map(lambda x: x + ' \\\n', code[:-1])) + [code[-1] + '\n'])
        else:
            code = ''
        self.update(self.makefile[:position_start], var, code, self.makefile[position_end:])

    def block_append(self, tag: str, content: list):
        code = self.block_get(tag)
        var = code[0] + '\n'
        code = code[1:] + content
        self.block_set(tag, code)

    def repair_multiple_definition(self, tag: str):
        self.block_set(tag, self.block_get(tag)[1:])

    def update_toolchain(self):
        self.replace('$(BINPATH)/', '$(BINPATH)')
        self.set_variable('BINPATH', '/opt/gcc-arm-none-eabi/bin/')

    def support_cpp(self):
        self.set_variable('CC', '$(BINPATH)$(PREFIX)g++')
        self.set_variable('CFLAGS', self.flags(CFLAGS))
        self.set_variable('LDFLAGS', self.flags(LDFLAGS))
        position = self.get_position_front(TAG_SOURCES_ASM)
        self.update(self.makefile[:position], TAG_SOURCES_CPP, '\nCPP_SOURCES = \\\n\n', self.makefile[position:])
        position = self.get_position(TAG_LIFT_OF_ASM_OBJECTS)
        self.update(self.makefile[:position], TAG_LIST_OF_CPP_OBJECTS, CMD_OBJECTS_APPEND_CPP, self.makefile[position:])
        position = self.get_position_front('$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)')
        self.update(self.makefile[:position], CMD_BUILD_CPP, self.makefile[position:])

    def alohal(self):
        position_start = self.get_position('-DSTM32F')
        position_end = self.get_position_behind('-DSTM32F')
        define = self.makefile[position_start:position_end].replace('F', '_F')
        self.block_append(TAG_DEFINES_C, [define])
        self.block_append(TAG_INCLUDES_C, ['-IALOHAL'])

    def hide_command(self, cmd: str):
        self.replace('\t' + cmd, '\t@' + cmd)

    def show_command(self, cmd: str):
        self.replace('\t@' + cmd, '\t' + cmd)

    def stm32_programmer(self):
        home = str(Path.home())
        result = subprocess.run(
            ['find', home, '-name', 'STM32_Programmer_CLI'],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        path = result.stdout.decode('utf8')[:-1]
        if path:
            position = self.get_position_front(TAG_EOF)
            self.update(self.makefile[:position], CMD_FLASH.format(path), self.makefile[position:])
            position = self.get_position_front(TAG_EOF)
            self.update(self.makefile[:position], CMD_BUILD_AND_FLASH.format(path), self.makefile[position:])
        else:
            raise ProgrammerNotFound

    def modify(self):
        try:
            self.unix_end_line()
            self.check_was_modified()
            self.update_toolchain()
            #self.support_cpp()
            #self.alohal()
            self.set_variable('OPT', '-Os')
            self.hide_command('$(CC)')
            self.hide_command('$(AS)')
            self.hide_command('$(CP)')
            self.hide_command('$(AR)')
            self.hide_command('$(SZ)')
            self.hide_command('$(HEX)')
            self.hide_command('$(BIN)')
            self.stm32_programmer()

        except NotExist as e:
            print(str(e), file=sys.stderr)

        except MakefileIsModified as e:
            print(str(e), file=sys.stderr)

        except ProgrammerNotFound as e:
            print(str(e), file=sys.stderr)

        finally:
            self.repair_multiple_definition(TAG_SOURCES_PATH)
            self.repair_multiple_definition(TAG_SOURCES_C)
            self.repair_multiple_definition(TAG_SOURCES_CPP)
            self.repair_multiple_definition(TAG_SOURCES_ASM)
            self.repair_multiple_definition(TAG_INCLUDES_C)
            self.repair_multiple_definition(TAG_DEFINES_C)
            self.repair_multiple_definition(TAG_DEFINES_ASM)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-f',
        '--file',
        dest='path',
        action='store',
        default='Makefile',
        help='destination makefile',
    )
    Makefile(parser.parse_args().path)
