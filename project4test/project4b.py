#import toolspath
from testing import Xv6Build, Xv6Test
from testing.runtests import main

class Test0(Xv6Test):
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

all_test = [TestBase]


main(Xv6Build, all_test)
