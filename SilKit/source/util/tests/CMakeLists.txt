# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_silkit_test(TestUtilsVectorView SOURCES vector_view_test.cpp)
add_silkit_test(TestUtilsSilSerDes SOURCES SilSerializerTest.cpp SilSerDesTest.cpp)
add_silkit_test(TestUtilsCommandlineParser SOURCES CommandlineParserTest.cpp LIBS SilKitUtil)
add_silkit_test(TestUtilsSynchronizedHandlers SOURCES SynchronizedHandlersTest.cpp LIBS SilKitUtil)
