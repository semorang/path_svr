{
    "targets":[
        {
            # "target_name":"optimal_svr", 
            "target_name":"libtraffic_node",
            # "target_name":"pedestrian_svr",
            # "target_name":"vehicle_svr",
            # "target_name":"path_svr",
            "sources":[
                "traffic_node.cc", 

                "test_node.cpp",

                "./mapinfo.cpp",
                "./shw_mapinfol.cpp",
                "./sti_cache_s.cpp",
                "./sti_ctrl_s.cpp",
                "./sti_index_ctrl.cpp",
                "./sti_reader_s.cpp",
                "./thfile.cpp",
                "./thholiday.cpp",
                #"./thlog.cpp",
                "./thmemalloc.cpp",
                "./thmemfile.cpp",
                "./thstring.cpp",
                "./thtime.cpp",
                "./thwinfile.cpp",
                "./ttl_mapinfo_r.cpp",
                "./ttl_mapreader_r.cpp",
                
                "./ReadTTLSpd.cpp",



                # # "../ext/libjson/json_reader.cpp",
                # # "../ext/libjson/json_writer.cpp",
                # # "../ext/libjson/json_value.cpp",
                # # "../ext/libjson/json_valueiterator.inl",       
                # "../ext/libjson/cjson/cJSON.c",

                # "../ext/shp/ShpObj.cpp",
                # "../ext/shp/ShpReader.cpp",
                # "../ext/shp/ShpWriter.cpp",

                # "../ext/mst/prim.cpp",
                # "../ext/mst/tsp_ga.cpp",
                # # "../ext/tsp/cross.cpp",
                # # "../ext/tsp/environment.cpp",
                # # "../ext/tsp/evaluator.cpp",
                # # "../ext/tsp/indi.cpp",
                # # "../ext/tsp/kopt.cpp",
                # # "../ext/tsp/randomize.cpp",
                # # "../ext/tsp/sort.cpp",

                # "../ext/tms/GnKmeansClassfy.cpp",

                # "../ext/route/MMPoint.hpp",
                # "../ext/route/MapBase.cpp",
                # "../ext/route/MapName.cpp",
                # "../ext/route/MapMesh.cpp",
                # "../ext/route/MapNode.cpp",
                # "../ext/route/MapLink.cpp",
                # "../ext/route/MapPolygon.cpp",
                # "../ext/route/MapTraffic.cpp",
                # "../ext/route/MapCourse.cpp",
                # "../ext/route/DataManager.cpp",
                # "../ext/route/TrafficManager.cpp",
                # "../ext/route/RoutePlan.cpp",
                # "../ext/route/RouteManager.cpp",
                # "../ext/route/RoutePackage.cpp",

                # "../ext/cvt/FileBase.cpp",
                # "../ext/cvt/FileManager.cpp",
                # "../ext/cvt/FileName.cpp",
                # "../ext/cvt/FileMesh.cpp",
                # "../ext/cvt/FilePedestrian.cpp",
                # "../ext/cvt/FileTrekking.cpp",
                # "../ext/cvt/FileForest.cpp",
                # "../ext/cvt/FileVehicle.cpp",
                # "../ext/cvt/FileBuilding.cpp",
                # "../ext/cvt/FileComplex.cpp",
                # "../ext/cvt/FileEntrance.cpp",
                # "../ext/cvt/FileMountain.cpp",
                # "../ext/cvt/FileTraffic.cpp"
            ],
            "conditions":[
                ['OS=="linux"', {
#                     "cflags":['-fexceptions',
#                         '-unused-result',
#                         '-fopenmp',
#                     ],
#                     "cflags_cc":['-fexceptions',
#                         '-unused-result',
#                         '-fopenmp',
#                     ],
#                     "ldflags": [
# #                      '-fopenmp',
#                    ],
#                    "libraries": [
#                         '-fopenmp',
#                    ],
                }],
                ['OS=="win"', {
                    # 'msvs_settings': {
                    #     'VCLinkerTool': {
                    #         'AdditionalOptions': [ '/NODEFAULTLIB:library' ],
                    #         'IgnoreDefaultLibraryNames': [
                    #             'nafxcw.lib', 'libcmtd.lib'
                    #     ]},
                    #     'VCCLCompilerTool': {
                    #         'AdditionalOptions': ['/MT', '/openmp'],
                    #     },
                    # },
                    # # 'include_dirs': ["../ext/libjson/include"],
                    # 'libraries': [
                    #         '-lnafxcw.lib', '-llibcmtd.lib',
                    #         # '-L<some library directory>'
                    # ],
                }],
            ],
            "defines":[
                # "USE_TECKING_DATA",
                # "USE_PEDESTRIAN_DATA",
                "_ENFORCE_MATCHING_ALLOCATORS=0"
            ],
        }
    ]
}
