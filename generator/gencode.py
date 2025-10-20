# -----------------------------------------------------------------------------
#  HSM-to-C++ conversion tool
#
#  The convertor class
#
#  Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see https://www.gnu.org/licenses/
#  -----------------------------------------------------------------------------

import sys
import traceback
import re
import os

import CyberiadaML

GLOBAL_INIT_LABEL = 'GLOBAL C INIT'
MAIN_C_INIT_LABEL = 'MAIN C INIT'
MAIN_INIT_LABEL = 'MAIN INIT'
MAIN_LOOP_LABEL = 'MAIN LOOP'
SM_VARIABLES_LABEL = 'SM Variables'
EVENT_VARIABLES_LABEL = 'Event Variables'

TEMPLATES = 'templates'
MAIN_CPP = 'main.cpp'

STANDARD_EVENTS = set(['TIME_TICK', 'TIME_TICK_SEC'])

def DEBUG(*args):
    sys.stderr.write(' '.join(map(lambda s: str(s), args)) + '\n')

class ConvertorError(Exception):
    def __init__(self, msg):
        Exception.__init__(self)
        self.msg = msg
    def __str__(self):
        return self.msg
class ParserError(ConvertorError):
    def __init__(self, msg):
        ConvertorError.__init__(self, msg)
class GeneratorError(ConvertorError):
    def __init__(self, msg):
        ConvertorError.__init__(self, msg)

class CodeGenerator:
    def __init__(self, graph_file):
        self.__load_graph(graph_file)

    def __load_graph(self, graph_file):
        try:
            self.__graph_file = graph_file
            self.__doc = CyberiadaML.LocalDocument()
            self.__doc.open(graph_file, CyberiadaML.formatDetect, CyberiadaML.geometryFormatNone,
                            False, False, True)
            self.__graph = self.__doc.get_state_machines()[0]

            self.__sm_name = self.__graph.get_name()
            self.__global_init = []
            self.__main_c_init = []
            self.__main_init = []
            self.__main_loop = []
            sm_variables = []
            self.__event_variables = []
            comment_labels = {
                GLOBAL_INIT_LABEL: self.__global_init,
                MAIN_C_INIT_LABEL: self.__main_c_init,
                MAIN_INIT_LABEL: self.__main_init,
                MAIN_LOOP_LABEL: self.__main_loop,
                SM_VARIABLES_LABEL: sm_variables,
                EVENT_VARIABLES_LABEL: self.__event_variables
            }

            self.__signals = set([])

            for comment in self.__graph.find_elements_by_type(CyberiadaML.elementComment):
                text = comment.get_body()
                for label, var in comment_labels.items():
                    if text.find(label) == 0:
                        for i, line in enumerate(text.splitlines()):
                            if i == 0:
                                continue
                            line = line.strip()
                            if len(line) == 0:
                                continue
                            var.append(line)
                        break

            self.__sm_variables = {}
            for smv in sm_variables:
                var, value = map(lambda s: s.strip(), smv.replace(';', '').split('='))
                self.__sm_variables[var] = value
                    
            init_id = None
            self.__initial = None
            for state in self.__graph.get_children():
                if state.get_type() == CyberiadaML.elementInitial:
                    if init_id is not None:
                        raise ParserError('The graph {} has more than one initial'.format(self.__graph_file) +
                                          'pseudostate on the top level!\n')
                    init_id = state.get_id()
            if init_id is None:
                raise ParserError('The graph {} has no initial pseudostate!\n'.format(self.__graph_file))
            uniq_states = set([])

            self.__transitions = []

            types = [CyberiadaML.elementTransition,
                     CyberiadaML.elementSimpleState,
                     CyberiadaML.elementCompositeState]
            for element in self.__graph.find_elements_by_types(types):
                if element.get_type() == CyberiadaML.elementTransition:
                    source_id = element.get_source_element_id()
                    if source_id == init_id:
                        target_id = element.get_target_element_id()
                        self.__initial = self.__graph.find_element_by_id(target_id)
                        continue
                    source_state = self.__graph.find_element_by_id(source_id)
                    if source_state.get_type() == CyberiadaML.elementInitial:
                        continue
                    a = element.get_action()
                    if len(a.get_trigger()) == 0:
                        raise ParserError('The graph {} has state {} with empty external transition!\n'.format(self.__graph_file,
                                                                                                               element.get_id()))
                    self.__check_trigger_and_behavior(element.get_id(), a.get_trigger(), a.get_guard(), a.get_behavior())
                    self.__signals.add(a.get_trigger())
                    self.__transitions.append(element)
                else:
                    state_name = element.get_name()
                    if len(state_name) == 0:
                        raise ParserError('The graph {} has state {} with empty name!\n'.format(self.__graph_file,
                                                                                                element.get_id()))
                    if state_name.find(' ') >= 0:
                        raise ParserError('The graph {} has state {} with spaces in name "{}"!\n'.format(self.__graph_file,
                                                                                                         element.get_id(),
                                                                                                         state_name))
                    full_name = element.get_qualified_name().replace('::', '_')
                    if full_name in uniq_states:
                        raise ParserError('The graph {} has two states with the same qualfied name {}!\n'.format(self.__graph_file,
                                                                                                                 full_name))
                    uniq_states.add(full_name)
                    for a in element.get_actions():
                        if a.get_type() == CyberiadaML.actionTransition:
                            if len(a.get_trigger()) == 0:
                                raise ParserError('The graph {} has state {} with empty trigger in int.trans.!\n'.format(self.__graph_file,
                                                                                                                         element.get_id()))
                            self.__check_trigger_and_behavior(full_name, a.get_trigger(), a.get_guard(), a.get_behavior())
                            self.__signals.add(a.get_trigger())
                        else:
                            self.__check_trigger_and_behavior(full_name, None, None, a.get_behavior())
            if self.__initial is None:
                raise ParserError('The game graph {} has no initial state!\n'.format(self.__graph_file))

            self.__templates = {
                '/*$$STATE_MACHINE_NAME$$*/': self.__sm_name,
                '/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/': self.__sm_name[0].upper() + self.__sm_name[1:],
                '/*$$GLOBAL_INIT$$*/': '\n'.join(self.__global_init),
            }

            for s in STANDARD_EVENTS:
                if s in self.__signals:
                    self.__signals.remove(s)

        except CyberiadaML.Exception as e:
            raise ParserError('Unexpected CyberiadaML exception: {}\n{}\n'.format(e.__class__,
                                                                                  traceback.format_exc()))

    def __check_trigger_and_behavior(self, context, trigger, guard, behavior):
        pass

    def __write_states_definitions_recursively(self, state, state_path, output):
        state_path = state_path + '::' + state.get_name()
        output.write('/* {} */\n'.format(state_path))
        self.__insert_string('QState /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_'+
                             '{}(/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ * const me, QEvt const * const e) {{\n'.format(state.get_name()),
                             output)
        self.__insert_string('    QState status_;\n', output)
        self.__insert_string('    switch (e->sig) {\n', output)
        self.__insert_string('        /* ' + state_path + ' */\n', output)
        self.__insert_string('        case Q_ENTRY_SIG: {\n', output)
        for a in state.get_actions():
            if a.get_type() == CyberiadaML.actionEntry and a.has_behavior():
                b = map(lambda s: s.strip(), a.get_behavior().split('\n'))
                self.__insert_string('\n'.join(map(lambda s: '            ' + s + '\n', b)), output)
                break
        self.__insert_string('            status_ = Q_HANDLED();\n', output)
        self.__insert_string('            break;\n', output)
        self.__insert_string('        }\n', output)
        self.__insert_string('        /*.${' + state_path + '} */\n', output)
        self.__insert_string('        case Q_EXIT_SIG: {\n', output)
        for a in state.get_actions():
            if a.get_type() == CyberiadaML.actionExit and a.has_behavior():
                b = map(lambda s: s.split(), a.get_behavior().split('\n'))
                self.__insert_string('\n'.join(map(lambda s: '            ' + s + '\n', b)), output)
                break
        self.__insert_string('            status_ = Q_HANDLED();\n', output)
        self.__insert_string('            break;\n', output)
        self.__insert_string('        }\n', output)

        triggers_merged = {}
        for a in state.get_actions():
            if a.get_type() == CyberiadaML.actionTransition:
                trigger = a.get_trigger()
                if trigger not in triggers_merged:
                    triggers_merged[trigger] = [(a, 'internal', None)]
                else:
                    triggers_merged[trigger].append((a, 'internal', None))
        for t in self.__transitions:
            if t.get_source_element_id() == state.get_id():
                action = t.get_action()
                trigger = action.get_trigger()
                if trigger not in triggers_merged:
                    triggers_merged[trigger] = [(action, 'external', t.get_target_element_id())]
                else:
                    triggers_merged[trigger].append((action, 'external', t.get_target_element_id()))

        for event_name, triggers in triggers_merged.items():
            self.__insert_string('        /*. {}::{} */\n'.format(state_path, event_name), output)
            self.__insert_string('        case {}_SIG: {{\n'.format(event_name), output)
            if len(triggers) == 1:
                action, _, _ = triggers[0]
                if action.get_guard():
                    self.__insert_string('            if ({}) {{\n'.format(action.get_guard()), output)
                    self.__write_trigger(triggers[0], state_path, event_name, '    ', output)
                    self.__insert_string('            }\n', output)
                    self.__insert_string('            else {\n', output)
                    self.__insert_string('                status_ = Q_UNHANDLED();\n', output)
                    self.__insert_string('            }\n', output)
                else:
                    self.__write_trigger(triggers[0], state_path, event_name, '', output)
            else:
                for i, trigger in enumerate(triggers):
                    if trigger[0].get_guard() == 'else' and i < len(triggers) - 1:
                        triggers[-1], triggers[i] = triggers[i], triggers[-1]
                        break
                action, _, _ = triggers[0]
                self.__insert_string('            if ({}) {{\n'.format(action.get_guard()), output)
                self.__write_trigger(triggers[0], state_path, event_name, '    ', output)
                self.__insert_string('            }\n', output)
                if len(triggers) >= 2:
                    for t in triggers[1:]:
                        action, _, _ = t
                        if action.get_guard() == 'else':
                            break
                        self.__insert_string('            else if ({}) {{\n'.format(action.get_guard()), output)
                        self.__write_trigger(t, state_path, event_name, '    ', output)
                        self.__insert_string('            }\n', output)
                if triggers[-1][0].get_guard() == 'else':
                    self.__insert_string('            else {\n', output)
                    self.__write_trigger(triggers[-1], state_path, event_name, '    ', output)
                    self.__insert_string('            }\n', output)
            self.__insert_string('            break;\n', output)
            self.__insert_string('        }\n', output)

        self.__insert_string('        default: {\n', output)
        if state.get_parent().get_type() != CyberiadaML.elementSM:
            self.__insert_string('            status_ = Q_SUPER(&/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_' +
                                 '{});\n'.format(state.get_parent().get_name()), output)
        else:
            self.__insert_string('            status_ = Q_SUPER(&QHsm_top);\n', output)
        self.__insert_string('            break;\n', output)
        self.__insert_string('        }\n', output)
        self.__insert_string('    }\n', output)
        self.__insert_string('    return status_;\n', output)
        self.__insert_string('}\n', output)

        for child in state.get_children():
            if child.get_type() in (CyberiadaML.elementSimpleState, CyberiadaML.elementCompositeState):
                self.__write_states_definitions_recursively(child, state_path, output)

    def __write_states_declarations_recursively(self, state, output):
        self.__insert_string('QState /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_' +
                             '{}(/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ * const me, QEvt const * const e);\n'.format(state.get_name()), output)
        for child in state.get_children():
            if child.get_type() in (CyberiadaML.elementSimpleState, CyberiadaML.elementCompositeState):
                self.__write_states_declarations_recursively(child, output)

    def __write_signals(self, output):
        enum_labels = map(lambda s: s + '_SIG', self.__signals)
        enum = 'enum PlayerSignals {\n'
        enum += '  TIME_TICK_SEC_SIG = Q_USER_SIG,\n\n'
        enum += '  TIME_TICK_SIG,\n'
        enum += ',\n  '.join(enum_labels)
        enum += ',\n\n  LAST_USER_SIG\n};\n'
        output.write(enum)

    def __write_trigger(self, trigger, state_path, event_name, offset, output):
        action, trigger_type, target_id = trigger
        if action.has_behavior():
            self.__insert_string('\n'.join([offset + '            ' + line for line in action.get_behavior().split('\n')]) + '\n', output)
        if trigger_type == 'internal':
            self.__insert_string(offset + '            status_ = Q_HANDLED();\n', output)
        elif trigger_type == 'external':
            target_name = self.__graph.find_element_by_id(target_id).get_name()
            self.__insert_string(offset + '            status_ = Q_TRAN(&/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/' +
                                 '_{});\n'.format(target_name), output)
        else:
            raise GeneratorError('Unknown trigger type: {}'.format(trigger_type))

    def __write_constructor(self, output):
        self.__insert_string('void /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_ctor(', output)
        if self.__sm_variables:
            self.__insert_string('\n    ' + ',\n    '.join(sorted(self.__sm_variables.keys())) + ')\n', output)
            self.__insert_string('{\n', output)
        else:
            self.__insert_string('void) {\n', output)
        self.__insert_string('    /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ *me = &/*$$STATE_MACHINE_NAME$$*/;\n', output)
        # constructor code
        # TODO
        self.__insert_string('\n', output)
        self.__insert_string('    QHsm_ctor(&me->super, Q_STATE_CAST(&/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_initial));\n', output)
        if self.__sm_variables:
            for key, value in self.__sm_variables.items():
                var = key.split(' ')[-1]
                self.__insert_string('    me->{} = {};\n'.format(var, var), output)
        self.__insert_string('}\n', output)

    def __write_initial(self, output):
        self.__insert_string('QState /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/' +
                             '_initial(/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ * const me, void const * const par) {\n', output)
        self.__insert_string('    /* SM::/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/::SM::initial\n', output)
        self.__insert_string('    {} */\n'.format(self.__initial.get_name()), output)
        self.__insert_string('    return Q_TRAN(&/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_{});\n'.format(self.__initial.get_name()), output)
        self.__insert_string('}\n', output)

    def __insert_file_template(self, tmpl_name, output):
        with open(os.path.join(TEMPLATES, tmpl_name)) as input_file:
            for line in input_file.readlines():
                self.__insert_string(line, output)

    def __insert_string(self, s, output):
        s = re.sub('[ ]*\n', '\n', s) # skip spaces at the end
        for temp, value in self.__templates.items():
            s = s.replace(temp, value)
        output.write(s)

    def __write_sm_to_file(self):
        with open('{}.cpp'.format(self.__sm_name), 'w') as f:
            self.__insert_file_template('header.cpp', f)
            self.__write_constructor(f)
            self.__write_initial(f)
            for s in self.__graph.get_children():
                if s.get_type() in (CyberiadaML.elementSimpleState, CyberiadaML.elementCompositeState):
                    self.__write_states_definitions_recursively(s, "SM::" + self.__sm_name, f)
            self.__insert_file_template('footer.cpp', f)
            # if self.notes_dict['raw_cpp_code']:
            #     self.__insert_string('\n//Start of c code from diagram\n')
            #     self.__insert_string('\n'.join(self.notes_dict['raw_cpp_code'].split('\n')[1:]) + '\n')
            #     self.__insert_string('//End of c code from diagram\n\n\n')

        with open('{}.h'.format(self.__sm_name), 'w') as f:
            self.__insert_file_template('header.h', f)
            # if self.notes_dict['raw_h_code']:
            #     self.__insert_string('//Start of h code from diagram\n')
            #     self.__insert_string('\n'.join(self.notes_dict['raw_h_code'].split('\n')[1:]) + '\n')
            #     self.__insert_string('//End of h code from diagram\n\n\n')
            self.__insert_string('typedef struct {\n', f)
            self.__insert_string('/* protected: */\n', f)
            self.__insert_string('    QHsm super;\n', f)
            self.__insert_string('\n', f)
            self.__insert_string('/* public: */\n', f)
            self.__insert_string('    ' + '\n    '.join(map(lambda s: s + ';', self.__sm_variables)), f)
            self.__insert_string('\n} /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/;\n\n', f)
            self.__insert_string('/* protected: */\n', f)
            self.__insert_string('QState /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_initial(/*$$STATE_MACHINE_CAPITALIZED_NAME$$*/' +
                                 ' * const me, void const * const par);\n', f)
            for s in self.__graph.get_children():
                if s.get_type() in (CyberiadaML.elementSimpleState, CyberiadaML.elementCompositeState):
                    self.__write_states_declarations_recursively(s, f)
            self.__insert_string('typedef struct /*$$STATE_MACHINE_NAME$$*/_QEvt {\n', f)
            self.__insert_string('    QEvt super;\n', f)
            self.__insert_string('    ' + '\n    '.join(self.__event_variables), f)
            self.__insert_string('\n} /*$$STATE_MACHINE_NAME$$*/_QEvt;\n\n', f)
            self.__write_signals(f)
            self.__insert_string('extern QHsm * const the_/*$$STATE_MACHINE_NAME$$*/; ' +
                                 '/* opaque pointer to the /*$$STATE_MACHINE_NAME$$*/ HSM */\n\n', f)
            self.__insert_string('void /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/_ctor(', f)
            if self.__sm_variables:
                self.__insert_string('\n    ' + ',\n    '.join(sorted(self.__sm_variables.keys())) + ');\n', f)
            else:
                self.__insert_string('void);\n', f)
            self.__insert_file_template('footer.h', f)

    def __create_main(self):
        modelname = self.__sm_name[0].lower() + self.__sm_name[1:]
        Modelname = self.__sm_name[0].upper() + self.__sm_name[1:]
        with open(MAIN_CPP, "w") as f:
            with open(os.path.join(TEMPLATES, MAIN_CPP)) as templ:
                templates = {
                    '/*$$INCLUDE$$*/': '#include "{}.h"'.format(modelname) + '\n',
                    '/*$$MACHINE$$*/': '"{} State Machine"'.format(Modelname),
                    '/*$$MAIN_C_INIT$$*/': '\n'.join(self.__main_c_init),
                    '/*$$MAIN_LOOP$$*/': '\n'.join(self.__main_loop),
                    '/*$$MAIN_INIT$$*/': '\n'.join(map(lambda s: '    ' + s, self.__main_init)),
                    '/*$$CONSTRUCTOR$$*/': "{}_ctor({});\n".format(Modelname,
                                                                   ', '.join(map(lambda x: x[1],
                                                                                 sorted(self.__sm_variables.items(),
                                                                                        key = lambda k: k[0])))),
                    '/*$$SM_CALL$$*/': "the_{}".format(modelname),
                    '/*$$EVENT$$*/': "{}_QEvt e;".format(modelname),
                }
                for line in templ.readlines():
                    for tkey, tvalue in templates.items():
                        line = line.replace(tkey, tvalue)
                    f.write(line)

    def generate_code(self):
        self.__write_sm_to_file()
        self.__create_main()

def usage():
    print('usage: {} <diagram.graphml>'.format(sys.argv[0]))
    exit(1)
        
if __name__ == '__main__':

    if len(sys.argv) != 2:
        usage()

    graph = sys.argv[1]

    try:
        g = CodeGenerator(graph)
        g.generate_code()
    except game.ParserError as e:
        sys.stderr.write('Graph parsing error: {}\n'.format(e))
        exit(1)
    except game.GeneratorError as e:
        sys.stderr.write('Code generating error: {}\n'.format(e))
        exit(2)
    except game.ConvertorError as e:
        sys.stderr.write('Strange convertor error: {}\n'.format(e))
        exit(3)
    except Exception as e:
        sys.stderr.write('Unexpected exception: {}\n'.format(e.__class__))
        sys.stderr.write('{}\n'.format(traceback.format_exc()))
        exit(4)

    exit(0)
