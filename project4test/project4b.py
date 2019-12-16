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

class UserTest(Xv6Test):
   name = "usertests"
   description = """usertests"""
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
      self.name = testname
      self.descrption = testname
      self.tester = self.name + ".c"
      self.timeout = 30
      self.point_value = 10
      self.make_qemu_args = "CPUS={}".format(cpunum)

test1 = myTest("usertests")

all_test = [test1]


main(Xv6Build, all_test)
