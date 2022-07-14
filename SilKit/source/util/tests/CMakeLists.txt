# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_silkit_test(TestUtilsSpan SOURCES SpanTest.cpp)
add_silkit_test(TestUtilsSilSerDes SOURCES SilSerializerTest.cpp SilSerDesTest.cpp)
add_silkit_test(TestUtilsCommandlineParser SOURCES CommandlineParserTest.cpp LIBS SilKitUtil)
add_silkit_test(TestUtilsSynchronizedHandlers SOURCES SynchronizedHandlersTest.cpp LIBS SilKitUtil)
