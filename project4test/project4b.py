#import toolspath
from testing import Xv6Build, Xv6Test
from testing.runtests import main

class Test0(Xv6Test):
   name = "clone3-test"
   description = """clone3-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test1(Xv6Test):
   name = "join2-test"
   description = """join2-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test2(Xv6Test):
   name = "join3-test"
   description = """join3-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test3(Xv6Test):
   name = "size-test"
   description = """size-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test4(Xv6Test):
   name = "stack-test"
   description = """stack-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test5(Xv6Test):
   name = "thread-test"
   description = """thread-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test6(Xv6Test):
   name = "thread2-test"
   description = """thread2-test"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class Test7(Xv6Test):
   name = "null"
   description = """null"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

class TestBase(Xv6Test):
   name = "base"
   description = """base"""
   tester = name + ".c"
   timeout = 30
   point_value = 10
   make_qemu_args = "CPUS=1"

#==============================
# additional_tests = [
#    myTest1, myTest2, myTest3, myTest5, myTest4
# ]

# released_tests = [ Test0, Test1, Test2, Test3, Test4, Test6, Test7, Test8, Test10 ]

class myTest(Xv6Test):
   def __init__(self, testname, cpunum=1):
      # super().__init__()
      name = testname
      descrption = testname
      tester = name + ".c"
      timeout = 30
      point_value = 10
      make_qemu_args = "CPUS={}".format(cpunum)

test1 = myTest("usertests")

all_test = [Test0, Test1, Test2, Test3, Test4, Test5, Test6]


main(Xv6Build, all_test)
