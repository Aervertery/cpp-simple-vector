#include "simple_vector.h"

#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include "tests.h"

int main() {
    Test1();
    Test2();
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    return 0;
}
