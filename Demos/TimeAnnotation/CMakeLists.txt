# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoTimeAnnotation TimeAnnotationDemo.cpp)

target_sources(SilKitDemoTimeAnnotation
    PRIVATE SilKitConfig_DemoTimeAnnotation.json
)
