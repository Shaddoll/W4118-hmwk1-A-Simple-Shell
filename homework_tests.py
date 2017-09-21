import os

import tester


GENERAL_ERROR = "[eE]rror:.*"

t = tester.HomeworkTester()

"""Modify this file with your tests.

The test is already filled out with some basic tests.

Basically, your main usage is:

	t.add_test("command to execute 1", "expected output as a regex string")
	t.add_test("command to execute 2", "expected output as a regex string")
	...
	t.add_test("command to execute 3", "expected output as a regex string")
	t.run()
	t.print_results()
	t.reset()
"""


##################### Basic Executables #########################
# ls should not be found
t.add_test("ls", GENERAL_ERROR)

# But /bin/echo should work
t.add_test("/bin/echo hello world", "hello world")
t.run()
t.print_results()
t.reset()

############################# Builtins ##########################
# Test that cd works
t.add_test("cd /tmp", "")
t.add_test("/bin/pwd", "/tmp")
t.add_test("cd /var", "")
t.add_test("/bin/pwd", "/var")
t.add_test("cd", GENERAL_ERROR)
t.add_test("cd /bin /var", "")
t.add_test("pwd", "/bin")
t.run()
t.print_results()
t.reset()

# Test that history works as expected
t.add_test("history", "0 history")
t.add_test("history -c", "")
t.add_test("    abc   abc   ", GENERAL_ERROR)
t.add_test("def", GENERAL_ERROR)
expected_output = [
    "0     abc   abc   |0 abc   abc   ",
    "1 def",
    "2 history",
]
t.add_test("history", "\n".join(expected_output))
t.add_test("history -c", "")
t.add_test("history", "0 history")
t.add_test("history blahblahblah", GENERAL_ERROR)
expected_output = [
        "0 history",
        "1 history blahblahblah",
        "2 history",
]
t.add_test("history", "\n".join(expected_output))
t.add_test("/bin/echo hello", "hello")

expected_output.extend(["3 /bin/echo hello", "4 history 0"])

t.add_test("history 0", "\n".join(expected_output))

expected_output.append("5 history")

t.add_test("history", "\n".join(expected_output))
t.add_test("history -c 0", "")
t.add_test("history", "0 history")
t.add_test("history 000", "\n".join(["0 history", "1 history 000"]))
t.add_test("history -c", "")
t.add_test("cd / | /bin/pwd", "/")
t.add_test("history 000 -c", "/")
t.add_test("history +0", GENERAL_ERROR)
t.add_test("history -c", "")
t.add_test("history blabla -c", GENERAL_ERROR)
t.add_test("history", "\n".join(["0 history blabla -c", "1 history"]))
t.run()
t.print_results()
t.reset()

############################# Pipes #############################
t.add_test("/bin/echo hello world | /bin/grep hello", "hello world")
t.add_test("/bin/echo blah          |/usr/bin/cut -b 3,4", "ah")
t.add_test("/bin/echo blah|/usr/bin/cut -b 3,4", "ah")
t.run()
t.print_results()
t.reset()

t.add_test("/bin/echo hello world", "hello world")
t.add_test("/bin/ls|history 0 | /usr/bin/wc -w", "2")
t.add_test("cd /", "")
t.add_test("/bin/ls|/bin/pwd", "/")
t.add_test("/bin/ls|/usr/bin/wc -w", "24")
t.add_test("/bin/ls|cd /", "")
t.add_test("cd /|/usr/bin/wc -w", "0")
t.run()
t.print_results()
t.reset()

t.add_test("history | /usr/bin/wc -w", "5")
t.add_test("cd /tmp | /bin/pwd", "/tmp")
t.run()
t.print_results()
t.reset()

############ Pipes with `balabla` commands ###############
t.add_test("cd /bin | blabla | cd ..", GENERAL_ERROR)
t.add_test("pwd", GENERAL_ERROR)
t.add_test("bin/pwd", "/")
t.run()
t.print_results()
t.reset()

###### History Loop ######
t.add_test("history 0", GENERAL_ERROR)
t.add_test("history -c", "")
t.add_test("history 1", GENERAL_ERROR)
t.add_test("history 0", GENERAL_ERROR)
t.run()
t.print_results()
t.reset()

#### History Loop with Pipe #####
t.add_test("cd /tmp |history 0 | cd ..", GENERAL_ERROR)
t.add_test("/bin/pwd", "/")
t.add_test("cd /var | history 0| cd /bin", GENERAL_ERROR)
t.add_test("pwd", "/bin")
t.add_test("history -c", "")
t.add_test("cd /var | history 2| cd /bin", GENERAL_ERROR)
t.add_test("pwd", "/bin")
t.add_test("cd /tmp|history 0|cd /usr", GENERAL_ERROR)
t.add_test("../bin/pwd", "/usr")
t.add_test("cd /bin | history 1 | cd /usr | history 0", GENERAL_ERROR)
t.add_test("cd /bin | history 2 | cd /usr | history 0", "\n".join([GENERAL_ERROR, GENERAL_ERROR]))
t.run()
t.print_results()
t.reset()

