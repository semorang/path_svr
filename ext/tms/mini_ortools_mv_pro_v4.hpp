// mini_ortools_mv_pro.hpp
// Multi-vehicle TSP/VRP (ATSP), K-nearest, Or-opt/2-opt/3-opt,
// Cross-exchange (k=1, generalized k=2), Relocate,
// Empty-route pruning, TW hard-mode repair,
// and O(1) delta evaluation for 2-opt/3-opt and small moves.
// MIT License (c) 2025

/* 사용법, by Gerniiie 20251014
void CMy3INAVIRoadNetGIConverterDlg::OnBnClickedButton16() { //TSP
    //auto vvvtResultLine = ConverRDMTable(g_vvvtResultLine); //table화 시킴..
    auto vvtEDA = ConverRDMTable(g_vvtEDA);
    //auto vvtETA = ConverRDMTable(g_vvtETA);

    printf("EDA(meter) table for TSP >>\n"); //이 내용을 TSP 알고리즘에 적용하면 된다..
    for (auto eda : vvtEDA) {
        printf("[");
        for (auto _eda : eda) {
            printf("%8.1f,", _eda);
        }
        printf("],\n");
    }
    printf("\n");

    vvtEDA(meter) table for TSP >>
[     0.0,   627.5,   397.8,   655.8,   882.3,   465.5,   455.1,   661.5,  1041.3,  1221.0,  1427.0,  1403.6,  1072.1,  1225.3,  1297.7,  1222.1,  1120.6,  1035.4,  1052.6,  1061.9,  1035.3,  1330.5,  1404.7,   823.0,  1350.8,   867.4,   948.7,  1052.6,  1227.8,  1239.8,  1607.7,  1748.8,  1231.8,  1287.2,  1246.6,  1351.8,],
[   345.1,     0.0,   229.1,   225.6,   159.1,   203.3,   193.0,   205.4,   876.1,  1055.8,  1261.9,  1238.5,   907.0,  1060.2,  1132.5,  1057.0,   955.4,   967.4,   887.5,   896.7,   967.6,  1099.4,  1654.6,   291.9,   313.7,  1117.2,   783.6,   887.4,  1062.7,  1074.7,  1442.5,  1400.7,  1066.7,  1214.1,  1173.5,  1278.7,],
[   115.5,   227.7,     0.0,   256.0,   482.5,    65.7,    55.3,   261.7,  1061.5,  1241.1,  1447.2,  1423.8,  1092.3,  1245.5,  1317.9,  1242.3,  1140.8,  1055.6,  1072.8,  1082.1,  1055.4,  1350.7,  1424.9,  1395.2,  1371.0,   887.6,   968.9,  1072.7,  1585.0,  1260.0,  1627.8,  1769.0,  1252.0,  1307.4,  1266.8,  1372.0,],
[   500.4,   154.9,   384.4,     0.0,   314.4,   358.6,   348.3,   127.9,   650.4,   830.1,  1036.2,  1012.8,   681.3,   834.4,   906.8,   831.2,   729.7,   741.7,   661.8,   671.0,   741.9,   873.7,  1809.9,    66.2,    88.0,  1272.5,   557.9,   661.7,   837.0,   849.0,  1216.8,  1175.0,   841.0,   988.4,   947.7,  1053.0,],
[   332.9,   254.4,   216.9,    99.4,     0.0,   191.1,   180.7,   133.1,   749.9,   929.6,  1135.6,  1112.3,   780.7,   933.9,  1006.3,   930.7,   829.2,   841.2,   761.3,   770.5,   841.3,   973.2,  1642.4,   165.7,   187.5,  1105.0,   657.4,   761.2,   936.5,   948.5,  1316.3,  1274.5,   940.4,  1087.8,  1047.2,  1152.4,],
[   183.1,   203.3,    67.1,   257.6,   458.1,     0.0,     0.0,   237.3,   983.1,  1162.8,  1514.8,  1491.5,  1159.9,  1167.1,  1385.5,  1163.9,  1208.4,  1123.3,  1140.5,  1149.7,  1123.1,  1418.4,  1492.6,   398.9,   420.7,   955.2,   890.6,   994.4,  1652.7,  1181.7,  1695.5,  1836.7,  1319.6,  1375.0,  1334.4,  1439.6,],
[   434.4,   355.8,   318.3,   200.9,   515.3,     0.0,     0.0,   234.6,   851.4,  1031.0,  1237.1,  1213.7,   882.2,  1035.4,  1107.8,  1032.2,   930.6,   942.6,   862.7,   871.9,   942.8,  1074.6,  1743.8,   267.1,   288.9,  1206.5,   758.8,   862.6,  1037.9,  1049.9,  1417.7,  1375.9,  1041.9,  1189.3,  1148.7,  1253.9,],
[   379.2,   205.4,   263.1,   199.7,   133.1,   237.3,   227.0,     0.0,   850.2,  1029.8,  1235.9,  1212.5,   881.0,  1034.2,  1106.6,  1031.0,   929.4,   941.4,   861.5,   870.7,   941.6,  1073.4,  1688.6,   265.9,   287.7,  1151.3,   757.6,   861.4,  1036.7,  1048.7,  1416.5,  1374.7,  1040.7,  1188.1,  1147.5,  1252.7,],
[   975.2,  1376.1,  1146.4,   735.4,   834.9,  1214.1,  1203.7,   837.3,     0.0,   179.6,   548.3,   571.6,   158.7,   183.9,   456.3,   256.2,    79.2,    91.2,   211.2,   148.5,    91.3,   289.1,   731.4,   701.6,   677.4,   942.0,   275.3,   379.2,   186.5,   198.4,   569.3,   432.4,   190.4,   245.8,   205.2,   310.4,],
[  1137.2,  1538.0,  1308.3,   897.3,   996.8,  1376.0,  1365.6,   999.2,   529.8,     0.0,   647.2,   670.6,   392.6,   713.8,    12.0,   618.1,   441.1,   356.0,   373.2,   382.4,   355.8,   651.1,   893.3,   863.6,   839.4,  1103.9,   437.3,   541.1,   348.4,     0.0,   390.2,   531.4,   552.4,   607.7,   567.1,   521.4,],
[  1188.7,  1589.5,  1359.8,   948.8,  1048.3,  1427.5,  1417.1,  1050.7,   581.3,   761.0,     0.0,     0.0,   444.1,   765.3,   131.7,   472.6,   295.6,   407.5,   424.7,   433.9,   407.3,   505.6,   944.8,   915.1,   890.8,  1155.4,   488.7,   395.6,   399.9,   779.9,  1150.7,   648.8,   406.8,   462.2,   421.6,   526.8,],
[   964.6,   942.7,  1224.8,   813.8,   913.3,  1292.5,  1282.1,   915.7,   446.3,   626.0,     0.0,     0.0,   237.1,   630.3,   634.7,   334.6,   157.6,   169.5,   289.6,   226.9,   169.7,   367.5,   809.7,   780.0,   755.8,  1020.4,   353.7,   257.5,   264.8,   644.8,   696.7,   510.8,   268.8,   324.2,   283.6,   388.8,],
[   921.1,  1322.0,  1092.3,   681.3,   780.7,  1159.9,  1149.6,   783.2,   313.8,   253.4,   622.1,   645.5,     0.0,   257.8,   402.2,   330.1,   153.1,    67.9,   157.1,     0.0,    67.7,   363.0,   677.2,   647.5,   623.3,   887.9,   221.2,   253.0,   132.3,   272.3,   643.1,   506.3,   264.3,   319.7,   279.1,   384.3,],
[  1154.1,  1555.0,  1325.3,   914.2,  1013.7,  1392.9,  1382.6,  1016.2,   546.7,    17.4,   664.1,   687.5,   409.6,     0.0,   672.1,   635.1,   458.0,   372.9,   390.1,   399.3,   372.7,   668.0,   910.2,   880.5,   856.3,  1120.8,   454.2,   558.0,   365.3,    36.3,   407.1,   548.3,   569.3,   624.7,   584.1,   538.3,],
[   971.4,   949.6,  1231.6,   820.6,   920.1,  1299.3,  1288.9,   922.5,   453.1,    11.3,   129.8,   106.4,   243.9,   637.1,     0.0,   341.4,   164.4,   176.4,   296.4,   233.7,   176.5,   374.3,   816.6,   786.9,   762.6,  1027.2,   360.5,   264.4,   271.7,    30.1,   703.5,   517.6,   275.6,   331.0,   290.4,   395.6,],
[  1854.1,  1832.2,  2870.3,  1703.2,  1802.7,  2937.9,  2927.6,  1805.1,  1135.7,   991.4,  1634.1,  1657.5,   970.6,   995.7,  1642.1,     0.0,   817.4,   903.0,  1023.1,   960.3,   903.2,   715.0,  1699.2,  1669.5,  1645.3,  1909.8,  1999.2,   991.0,  1910.3,  1010.3,  1377.1,  1518.3,   774.3,   797.7,   757.0,  1508.3,],
[   969.6,   858.7,  1140.8,   729.7,   829.2,  1208.4,  1198.1,   831.6,   362.2,   173.9,   579.9,   556.6,   153.1,   178.2,   450.6,   343.0,     0.0,    85.5,   205.6,   142.8,    85.7,   385.5,   725.7,   696.0,   671.8,   936.3,   269.7,   173.5,   180.8,   192.8,   563.6,   704.8,   476.8,   500.2,   459.5,   564.8,],
[   892.5,   870.7,  1152.7,   741.7,   841.2,  1220.4,  1210.0,   843.6,   715.2,   185.9,   554.6,   578.0,   184.6,   190.2,   562.6,   262.5,    85.5,     0.0,   558.5,   194.8,     0.0,   295.5,   737.7,   708.0,   683.7,   948.3,   622.6,   185.5,   533.8,   204.7,   575.6,   438.7,   196.7,   252.1,   211.5,   316.7,],
[  1244.6,  1480.8,  1251.1,   941.1,  1040.5,  1318.7,  1308.4,  1514.8,   727.5,   907.2,  1103.3,  1079.9,   758.4,   911.6,   973.9,   908.4,   806.8,   818.8,     0.0,   748.1,   819.0,   950.8,  1609.0,  1579.3,   929.1,  2149.7,   635.0,   738.8,  1064.1,   926.1,  1283.9,  1425.1,   918.1,  1065.5,  1024.9,  1130.1,],
[   910.9,  1311.7,  1082.1,   671.0,   770.5,  1149.7,  1139.4,   772.9,   303.5,   243.2,   611.9,   635.3,     0.0,   247.5,   391.9,   319.8,   142.8,    57.7,   146.9,     0.0,    57.5,   352.8,   667.0,   637.3,   613.1,   877.6,   211.0,   242.8,   122.1,   262.1,   632.9,   496.1,   254.1,   309.5,   268.8,   374.1,],
[   884.3,  1285.1,  1055.4,   644.4,   743.9,  1123.1,  1112.7,   746.3,   276.9,   456.6,   494.6,   471.3,    67.7,   460.9,   365.3,   457.7,   356.2,     0.0,   120.3,    57.5,     0.0,   500.2,   640.4,   610.7,   586.5,   851.0,   184.3,   288.2,    95.5,   475.5,   675.3,   816.5,   467.4,   614.8,   574.2,   679.4,],
[  1179.5,  1068.6,  1350.7,   939.7,  1039.2,  1418.4,  1408.0,  1041.6,   572.2,   383.9,   492.6,   515.9,   363.0,   388.2,   500.6,    42.5,   209.9,   295.5,   415.5,   352.8,   295.6,     0.0,   935.6,   905.9,   881.7,  1146.3,   479.6,   383.5,   390.7,   402.7,   436.6,   376.7,   167.7,   190.1,   149.5,   254.7,],
[   858.4,  1094.6,   864.9,   751.9,   851.4,   932.6,   922.2,   853.8,   384.4,   564.1,   770.1,   746.7,   415.2,   568.4,   640.8,   565.2,   463.7,   378.5,   395.7,   405.0,   378.4,   673.6,     0.0,   718.1,   693.9,   210.5,   291.8,   395.7,   570.9,   582.9,   950.8,  1091.9,   574.9,   630.3,   589.7,   694.9,],
[   580.4,   816.6,   586.9,   844.9,  1071.4,   654.6,   644.2,   850.6,  1390.4,  1181.1,  1248.1,  1224.7,  1032.2,  1185.4,  1118.8,  1257.7,  1080.7,   995.5,  1233.7,  1022.0,   995.4,  1290.6,  1753.9,     0.0,    53.4,  1485.5,  1297.8,  1401.7,  1208.9,  1199.9,  1428.8,  1569.9,  1191.9,  1247.3,  1206.7,  1311.9,],
[   583.9,   166.4,   590.4,    37.4,   136.9,   370.1,   359.7,   139.3,   541.9,   721.6,   927.6,   904.2,   572.7,   725.9,   798.3,   722.7,   621.2,   633.2,   553.3,   562.5,   633.3,   765.2,  1273.4,     3.7,     0.0,  1489.0,   449.3,   553.2,   728.5,   740.5,  1108.3,  1066.4,   732.4,   879.8,   839.2,   944.4,],
[   692.2,   670.3,  1094.4,   541.4,   640.9,   874.1,   863.7,   643.3,   173.9,   353.6,   559.6,   536.2,   204.7,   357.9,   430.3,   354.7,   253.2,   168.0,   185.2,   194.5,   167.9,   463.1,   537.3,   507.6,   483.4,     0.0,    81.3,   185.2,   360.4,   372.4,   740.3,   881.4,   364.4,   419.8,   379.2,   484.4,],
[  1067.5,  1468.4,  1238.7,   827.7,   927.1,  1306.3,  1296.0,   929.6,    92.2,   271.8,   732.5,   755.9,   251.0,   276.2,   548.6,   273.0,   171.4,   183.4,   303.5,   240.7,   183.6,   315.4,   823.6,   793.9,   769.7,  1034.3,     0.0,   103.4,   278.7,   290.7,   661.5,   616.7,   282.7,   430.1,   389.5,   494.7,],
[  1315.6,  1204.7,  1486.7,  1075.7,  1175.2,  1554.4,  1544.0,  1177.6,   708.2,   519.9,   628.6,   652.0,   499.0,   524.2,   636.6,   169.0,   345.9,   431.5,   551.6,   488.8,   431.7,   211.5,  1071.7,  1042.0,  1017.8,  1282.3,   615.7,     0.0,   526.8,   538.8,  1451.6,   512.8,   302.8,   326.1,   285.5,   390.8,],
[   789.4,  1190.2,   960.6,   549.5,   649.0,  1028.2,  1017.9,   651.4,   182.0,   361.7,   822.4,   845.8,   340.9,   366.0,   638.4,   362.8,   261.3,   273.3,   393.4,   330.6,   273.5,   405.3,   545.5,   515.8,   491.6,   756.1,    89.5,   193.3,     0.0,   380.6,   751.4,   706.6,   372.6,   520.0,   479.3,   584.6,],
[  1118.3,  1519.2,  1289.5,   878.5,   977.9,  1357.1,  1346.8,   980.4,   510.9,     0.0,   628.3,   651.7,   373.8,   695.0,    30.9,   599.3,   422.2,   337.1,   354.3,   363.5,   336.9,   632.2,   874.4,   844.7,   820.5,  1085.1,   418.4,   522.2,   329.5,     0.0,   371.3,   512.5,   533.5,   588.9,   548.3,   502.5,],
[  1321.9,  1722.7,  1493.0,  1082.0,  1181.5,  1560.7,  1550.3,  1183.9,   714.5,   894.2,   256.9,   280.3,   577.3,   898.5,   264.9,   296.8,   306.2,   540.7,   557.9,   567.1,   540.5,   329.8,  1078.0,  1048.3,  1024.1,  1288.6,   621.9,   528.8,   533.1,   913.1,     0.0,   141.1,   540.0,   139.4,   180.2,   131.0,],
[  1180.9,  1581.8,  1352.1,   941.1,  1040.5,  1419.7,  1409.4,  1043.0,   573.6,   753.2,   115.9,   139.3,   436.4,   757.6,   124.0,   464.9,   287.8,   399.7,   416.9,   426.1,   399.5,   497.8,   937.0,   907.3,   883.1,  1147.7,   481.0,   387.8,   392.1,   772.1,   277.4,     0.0,   399.1,   454.5,   413.9,   519.1,],
[  1048.5,   937.6,  1219.6,   808.6,   908.1,  1287.3,  1276.9,   910.5,   441.1,   252.8,   393.5,   416.9,   231.9,   257.1,   401.5,   101.4,    78.8,   164.4,   284.5,   221.7,   164.5,   135.4,   804.6,   774.9,   750.6,  1015.2,   348.5,   252.4,   259.7,   271.7,   463.5,   277.6,     0.0,    91.0,    50.4,   155.6,],
[  1367.2,  1768.1,  1538.4,  1127.3,  1226.8,  1606.0,  1595.7,  1229.3,   759.8,   939.5,   302.2,   325.6,   622.7,   943.9,   310.2,   651.2,   474.1,   586.0,   603.2,   612.4,   585.8,   684.1,  1123.3,  1093.6,  1069.4,  1333.9,   667.3,   574.1,   578.4,   958.4,  1329.2,   186.4,   585.4,     0.0,   600.2,    64.4,],
[  1408.0,  1808.8,  1579.1,  1168.1,  1267.6,  1646.8,  1636.5,  1270.0,   800.6,   980.3,   343.0,   366.4,   663.5,   984.6,   351.0,   210.9,   514.9,   626.8,   644.0,   653.2,   626.6,   243.9,  1164.1,  1134.4,  1110.2,  1374.7,   708.1,   614.9,   619.2,   999.2,   435.0,   227.2,   626.2,    40.6,     0.0,   105.2,],
[  1200.8,  1089.9,  1372.0,   961.0,  1060.4,  1439.6,  1429.3,  1062.9,   593.5,   405.1,   293.8,   317.2,   384.3,   409.5,   301.9,   221.8,   231.1,   316.7,   436.8,   374.1,   316.9,   254.7,   956.9,   927.2,   903.0,  1167.6,   500.9,   404.7,   412.0,   424.0,   583.8,   178.0,   188.0,    64.4,   105.2,     0.0,],


void CMy3INAVIRoadNetGIConverterDlg::OnBnClickedButton16() { //TSP
    //auto vvvtResultLine = ConverRDMTable(g_vvvtResultLine); //table화 시킴..
    auto vvtEDA = ConverRDMTable(g_vvtEDA);
    //auto vvtETA = ConverRDMTable(g_vvtETA);

    printf("EDA(meter) table for TSP >>\n"); //이 내용을 TSP 알고리즘에 적용하면 된다..
    for (auto eda : vvtEDA) {
        printf("[");
        for (auto _eda : eda) {
            printf("%8.1f,", _eda);
        }
        printf("],\n");
    }
    printf("\n");

#if 1
    printf("g_vvvtResultLine table>>\n");
    auto vvvtResultLine = ConverRDMTable(g_vvvtResultLine);
    for (auto eta : vvvtResultLine) { //경로선이 필요한경우..
        printf("[");
        for (auto _eta : eta) {
            printf("%5d,", _eta.size());
        }
        printf("],\n");
    }
    printf("\n");
#endif

    //or-tool 방식 적용
    using namespace operations_research;

    // ------------------------------------------------------------
    // [1] Distance Matrix 생성
    // ------------------------------------------------------------
    std::vector<std::vector<int>> dm(vvtEDA.size());
    for (int i = 0; i < (int)vvtEDA.size(); ++i) {
        dm[i].resize(vvtEDA[i].size());
        for (int j = 0; j < (int)vvtEDA[i].size(); ++j)
            dm[i][j] = static_cast<int>(vvtEDA[i][j] * 100.0); // scaling
    }

    // ------------------------------------------------------------
    // [2] 모드 설정
    // ------------------------------------------------------------
    enum class Mode { Circuit, PathFixedEnd, PathOpenEnd };

    Mode mode = Mode::PathFixedEnd; // ★ 원하는 모드 선택 (Circuit / PathFixedEnd / PathOpenEnd)
    int start_node = 0;
    int end_node = 2; // PathFixedEnd일 때만 의미 있음

    // ------------------------------------------------------------
    // [3] RoutingIndexManager / RoutingModel 구성
    // ------------------------------------------------------------
    int n_orig = (int)dm.size();
    int dummy_end_index = -1;
    bool use_dummy_end = (mode == Mode::PathOpenEnd);

    if (use_dummy_end) {
        dummy_end_index = n_orig;
        // 기존 행에 END 열 추가(0)
        for (auto& row : dm) row.push_back(0);
        // END 행 추가(의미 없음)
        dm.push_back(std::vector<int>(n_orig + 1, 0));
    }

    std::unique_ptr<RoutingIndexManager> upM;
    std::unique_ptr<RoutingModel> upR;

    if (mode == Mode::Circuit) {
        upM.reset(new RoutingIndexManager((int)dm.size(), 1, RoutingIndexManager::NodeIndex(start_node))); //1은 차량 대수..
    }
    else if (mode == Mode::PathFixedEnd) {
        std::vector<RoutingIndexManager::NodeIndex> starts = { RoutingIndexManager::NodeIndex(start_node) };
        std::vector<RoutingIndexManager::NodeIndex> ends = { RoutingIndexManager::NodeIndex(end_node) };
        upM.reset(new RoutingIndexManager((int)dm.size(), 1, starts, ends));
    }
    else if (mode == Mode::PathOpenEnd) {
        std::vector<RoutingIndexManager::NodeIndex> starts = { RoutingIndexManager::NodeIndex(start_node) };
        std::vector<RoutingIndexManager::NodeIndex> ends = { RoutingIndexManager::NodeIndex(dummy_end_index) };
        upM.reset(new RoutingIndexManager((int)dm.size(), 1, starts, ends));
    }

    RoutingIndexManager& M = *upM;
    upR.reset(new RoutingModel(M));
    RoutingModel& R = *upR;

    // ------------------------------------------------------------
    // [4] 비용 콜백 등록 (ATSP 가능)
    // ------------------------------------------------------------
    int64_t cb = R.RegisterTransitCallback(std::function<int64_t(int64_t, int64_t)>([&](int64_t a, int64_t b) {
        return (int64_t)dm[(int)a][(int)b]; })
    );

    R.SetArcCostEvaluatorOfAllVehicles(cb);
    // ------------------------------------------------------------
    // [5] 탐색 파라미터 설정
    // ------------------------------------------------------------
    RoutingSearchParameters p;
    p.candidate_k = 24;
    p.ls_rounds = 10;
    p.use_gls = false;
    p.use_three_opt = true;
    p.use_cross_exchange_k2 = false;
    p.enforce_candidate_bias = true;
    p.noncand_penalty_ratio = 0.25;
    p.use_oropt = true;
    p.use_relocate = true;

    // ------------------------------------------------------------
    // [6] Solve
    // ------------------------------------------------------------
    const Assignment* sol = R.SolveWithParameters(p);
    if (!sol) {
        std::printf("No solution\n");
        return;
    }

    // ------------------------------------------------------------
    // [7] 결과 출력
    // ------------------------------------------------------------
    g_vtBestVisitOrder.clear();
    g_vtBestVisitOrder.reserve(M.num_nodes());

    for (int v = 0; v < M.num_vehicles(); ++v) {
        std::printf("Route[%d]: ", v);
        int64_t idx = R.Start(v);
        while (!R.IsEnd(idx)) {
            int node = M.IndexToNode(idx).value();
            if (!(use_dummy_end && node == dummy_end_index)) {
                g_vtBestVisitOrder.push_back(node);
                std::printf("%d -> ", node);
            }
            idx = sol->Value(R.NextVar(idx));
        }

        int last_node = M.IndexToNode(idx).value();
        if (use_dummy_end)
            std::printf("END(dummy)\n");
        else {
            g_vtBestVisitOrder.push_back(last_node);
            std::printf("%d\n", last_node);
        }
    }

    Redraw();
}
*/


#pragma once
#include <vector>
#include <functional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <random>
#include <unordered_set>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace operations_research {

    // ----- API surface (OR-Tools 스타일 최소 모사) -----
    struct FirstSolutionStrategy { enum Value { PATH_CHEAPEST_ARC = 0 }; };

    struct RoutingSearchParameters {
        void set_first_solution_strategy(FirstSolutionStrategy::Value) {}

        // Candidate preprocessing
        int  candidate_k = 32;       // 0이면 전체 스캔
        bool shuffle_starts = false;

        // Constraints
        bool allow_soft_tw = true;        // false면 시간창 엄수 + 수리 알고리즘 사용
        bool forbid_cap_violation = true; // true면 용량 위반 금지

        // Local search
        int  ls_rounds = 7;
        int  two_opt_span_limit = 4096;
        bool use_oropt = true;
        bool use_two_opt = true;
        bool use_three_opt = true;
        bool use_cross_exchange_k1 = true;
        bool use_cross_exchange_k2 = true;  // ✅ k=2 활성화
        bool use_relocate = true;

        // GLS
        bool   use_gls = true;
        int    gls_iters = 5;
        double gls_lambda = 0.15;

        // Repair (TW hard mode)
        int repair_max_iters = 8;

        bool enforce_candidate_bias = true;   // 후보 간선 선호
        double noncand_penalty_ratio = 0.25;  // 비후보 간선에 얹는 가산비(평균 간선비용의 0.25배)
    };

    // --- Compatibility helpers (some forks may miss certain fields) ---
    template <typename P>
    auto __get_use_relocate_impl(const P& p, int) -> decltype((void)p.use_relocate, bool()) { return p.use_relocate; }
    template <typename P>
    bool __get_use_relocate_impl(const P&, ...) { return true; } // default: enabled

    inline bool GetUseRelocate(const RoutingSearchParameters& p) { return __get_use_relocate_impl(p, 0); }


    class RoutingIndexManager {
    public:
    class NodeIndex { public: explicit NodeIndex(int v = -1) :v_(v) {} int value() const { return v_; } private:int v_; };

                            RoutingIndexManager(int num_nodes, int num_vehicles, NodeIndex depot)
                                : n_(num_nodes), starts_(num_vehicles, depot.value()), ends_(num_vehicles, depot.value()) {
                            }
                            RoutingIndexManager(int num_nodes, int num_vehicles,
                                const std::vector<NodeIndex>& starts,
                                const std::vector<NodeIndex>& ends)
                                : n_(num_nodes), starts_(num_vehicles, 0), ends_(num_vehicles, 0) {
                                assert((int)starts.size() == num_vehicles && (int)ends.size() == num_vehicles);
                                for (int v = 0; v < num_vehicles; ++v) { starts_[v] = starts[v].value(); ends_[v] = ends[v].value(); }
                            }

                            int num_nodes() const { return n_; }
                            int num_vehicles() const { return (int)starts_.size(); }
                            const std::vector<int>& starts() const { return starts_; }
                            const std::vector<int>& ends()   const { return ends_; }

                            NodeIndex IndexToNode(int64_t gidx) const {
                                if (gidx < 0 || gidx >= (int64_t)global_order_.size()) return NodeIndex(starts_[0]);
                                return NodeIndex(global_order_[(size_t)gidx]);
                            }

                            void __set_global(const std::vector<int>& ord,
                                const std::vector<std::pair<int64_t, int64_t>>& spans) {
                                global_order_ = ord; vehicle_spans_ = spans;
                            }
                            const std::vector<int>& __order() const { return global_order_; }
                            const std::vector<std::pair<int64_t, int64_t>>& __vehicle_spans() const { return vehicle_spans_; }

    private:
        int n_{ 0 }; std::vector<int> starts_, ends_;
        std::vector<int> global_order_;
        std::vector<std::pair<int64_t, int64_t>> vehicle_spans_;
    };

    class Assignment {
    public:
        Assignment(std::vector<int64_t> nxt, int64_t obj) :next_(std::move(nxt)), obj_(obj) {}
        int64_t ObjectiveValue() const { return obj_; }
        int64_t Value(int64_t next_var) const {
            if (next_var < 0 || next_var >= (int64_t)next_.size()) return next_.size();
            return next_[(size_t)next_var];
        }
    private:
        std::vector<int64_t> next_; int64_t obj_;
    };

    class RoutingModel {
    public:
        explicit RoutingModel(RoutingIndexManager& m)
            : M_(m), N_(m.num_nodes()), V_(m.num_vehicles()),
            demand_(N_, 0), tw_early_(N_, 0),
            tw_late_(N_, std::numeric_limits<int64_t>::max() / 4),
            service_(N_, 0), cap_(V_, std::numeric_limits<int64_t>::max() / 4),
            t0_(V_, 0) {
        }

        int RegisterTransitCallback(std::function<int64_t(int64_t, int64_t)> cb) { dist_ = std::move(cb); return 0; }
        void SetArcCostEvaluatorOfAllVehicles(int) {}
        int RegisterTransitTimeCallback(std::function<int64_t(int64_t, int64_t)> cb) { time_ = std::move(cb); return 1; }

        void SetVehicleCapacity(int v, int64_t c) { cap_[v] = c; }
        void SetNodeDemand(int n, int64_t d) { if (valid(n)) demand_[n] = d; }
        void SetNodeTimeWindow(int n, int64_t e, int64_t l, int64_t s = 0) {
            if (!valid(n)) return; tw_early_[n] = e; tw_late_[n] = l; service_[n] = s;
        }
        void SetStartTime(int v, int64_t t) { t0_[v] = t; }

        const Assignment* SolveWithParameters(const RoutingSearchParameters& p) {
            if (!dist_) return nullptr;

            params_ = p;
            // 평균 간선비 계산 (대충으로도 충분)
            long double sum = 0; long long cnt = 0;
            for (int i = 0; i < N_; ++i) for (int j = 0; j < N_; ++j) if (i != j) { sum += dist(i, j); ++cnt; }
            avg_edge_cost_ = (cnt ? (double)(sum / cnt) : 0.0);

            build_candidates(p.candidate_k);

            std::vector<std::vector<int>> routes;
            build_initial_routes(routes, p);

            if (!p.allow_soft_tw) repair_time_windows(routes, p);

            int64_t best = total_cost(routes, true);
            if (p.use_gls) guided_local_search(routes, best, p);

            local_search(routes, best, p);

            prune_empty_routes(routes);

            // global order & spans
            std::vector<int> ord; ord.reserve(aggregate_size(routes));
            std::vector<std::pair<int64_t, int64_t>> spans;
            for (auto& r : routes) {
                if (r.size() < 2) continue;
                int64_t s = (int64_t)ord.size();
                ord.insert(ord.end(), r.begin(), r.end());
                int64_t e = (int64_t)ord.size() - 1;
                spans.emplace_back(s, e);
            }
            M_.__set_global(ord, spans);

            // next 배열
            const int64_t G = (int64_t)ord.size();
            std::vector<int64_t> next(G, G);
            for (auto sp : spans) for (int64_t i = sp.first; i < sp.second; ++i) next[i] = i + 1;

            assigns_.emplace_back(std::move(next), best);
            return &assigns_.back();
        }

        // Cursor
        int64_t Start(int v) const { return M_.__vehicle_spans()[v].first; }
        bool IsEnd(int64_t idx) const {
            for (auto& sp : M_.__vehicle_spans()) if (idx == sp.second) return true; return false;
        }
        int64_t NextVar(int64_t i) const { return i; }

    private:
        // ---------- 기본 유틸 ----------
        bool valid(int n) const { return n >= 0 && n < N_; }
        inline int64_t dist(int a, int b) const { return dist_(a, b); }
        inline int64_t tcost(int a, int b) const { return time_ ? time_(a, b) : dist_(a, b); }
        size_t eid(int a, int b) const { return (size_t)a * (size_t)N_ + (size_t)b; }

        struct Feas { int64_t cap_violation = 0, late_pen = 0, wait_pen = 0; bool feasible = true; };

        Feas eval_route(int v, const std::vector<int>& r) const {
            Feas f; int64_t load = 0, t = t0_[v];
            for (size_t i = 0; i < r.size(); ++i) {
                int u = r[i];
                load += demand_[u];
                if (load > cap_[v]) { f.cap_violation += (load - cap_[v]); f.feasible = false; }
                if (t < tw_early_[u]) t = tw_early_[u];
                if (t > tw_late_[u]) { f.late_pen += (t - tw_late_[u]); f.feasible = false; }
                t += service_[u];
                if (i + 1 < r.size()) t += tcost(u, r[i + 1]);
            }
            return f;
        }

        int64_t route_cost_edges_only(const std::vector<int>& r, bool with_gls) const {
            int64_t c = 0;
            for (size_t i = 0; i + 1 < r.size(); ++i) {
                c += dist(r[i], r[i + 1]);
                if (with_gls && !gls_pen_.empty()) c += (int64_t)(gls_lambda_ * gls_pen_[eid(r[i], r[i + 1])]);
            }
            return c;
        }

        int64_t total_cost(const std::vector<std::vector<int>>& routes, bool with_pen) const {
            int64_t c = 0;
            for (int v = 0; v < (int)routes.size(); ++v) {
                auto& r = routes[v]; if (r.size() < 2) continue;
                c += route_cost_edges_only(r, /*with_gls=*/true);
                Feas f = eval_route(v, r);
                if (with_pen) {
                    if (f.cap_violation > 0) c += (int64_t)1e12 * f.cap_violation;
                    c += f.late_pen;
                }
            }
            return c;
        }

        // ---------- 후보 전처리 ----------
        void build_candidates(int K) {
            if (K <= 0) { cand_.assign(N_, {}); gls_pen_.assign((size_t)N_ * (size_t)N_, 0); gls_lambda_ = 0; return; }
            cand_.assign(N_, {}); gls_pen_.assign((size_t)N_ * (size_t)N_, 0); gls_lambda_ = 0;
            for (int u = 0; u < N_; ++u) {
                std::vector<std::pair<int64_t, int>> w; w.reserve(N_ - 1);
                for (int v = 0; v < N_; ++v) if (u != v) w.emplace_back(dist(u, v), v);
                int kk = std::min(K, (int)w.size());
                std::nth_element(w.begin(), w.begin() + kk, w.end(), [](auto& a, auto& b) {return a.first < b.first; });
                w.resize(kk);
                std::sort(w.begin(), w.end(), [](auto& a, auto& b) {return a.first < b.first; });
                cand_[u].reserve(kk);
                for (auto& x : w) cand_[u].push_back(x.second);
            }
        }

        // ---------- 초기 해 ----------
        void build_initial_routes(std::vector<std::vector<int>>& routes, const RoutingSearchParameters& p) {
            const auto& S = M_.starts(); const auto& E = M_.ends();
            routes.assign(V_, {});
            for (int v = 0; v < V_; ++v) { routes[v].push_back(S[v]); routes[v].push_back(E[v]); }

            std::vector<char> used((size_t)N_, 0);
            for (int v = 0; v < V_; ++v) { used[(size_t)S[v]] = 1; used[(size_t)E[v]] = 1; }
            std::vector<int> cust; cust.reserve(N_);
            for (int i = 0; i < N_; ++i) {
                bool terminal = false; for (int v = 0; v < V_; ++v) if (i == S[v] || i == E[v]) { terminal = true; break; }
                if (!terminal) cust.push_back(i);
            }

            size_t ci = 0;
            while (ci < cust.size()) {
                bool any = false;
                for (int v = 0; v < V_ && ci < cust.size(); ++v) {
                    int node = cust[ci]; if (used[(size_t)node]) { ++ci; continue; }
                    int best_pos = -1; int64_t best_delta = std::numeric_limits<int64_t>::max();
                    auto& r = routes[v];
                    for (size_t i = 0; i + 1 < r.size(); ++i) {
                        int a = r[i], b = r[i + 1];
                        int64_t delta = (dist(a, node) + dist(node, b) - dist(a, b));
                        auto cand = r; cand.insert(cand.begin() + i + 1, node);
                        Feas f = eval_route(v, cand);
                        if (p.forbid_cap_violation && f.cap_violation > 0) continue;
                        if (!p.allow_soft_tw && f.late_pen > 0) continue;
                        delta += f.late_pen + (int64_t)1e12 * f.cap_violation;
                        if (delta < best_delta) { best_delta = delta; best_pos = (int)i + 1; }
                    }
                    if (best_pos >= 0) { routes[v].insert(routes[v].begin() + best_pos, node); used[(size_t)node] = 1; ++ci; any = true; }
                }
                if (!any) {
                    // 완화 삽입
                    int best_v = -1, best_pos = -1; int64_t best = std::numeric_limits<int64_t>::max();
                    for (int v = 0; v < V_; ++v) {
                        auto& r = routes[v];
                        for (size_t i = 0; i + 1 < r.size(); ++i) {
                            int node = cust[ci];
                            int64_t delta = dist(r[i], node) + dist(node, r[i + 1]) - dist(r[i], r[i + 1]);
                            auto cand = r; cand.insert(cand.begin() + i + 1, node);
                            Feas f = eval_route(v, cand);
                            delta += f.late_pen + (int64_t)1e12 * f.cap_violation;
                            if (delta < best) { best = delta; best_v = v; best_pos = (int)i + 1; }
                        }
                    }
                    if (best_v >= 0) { routes[best_v].insert(routes[best_v].begin() + best_pos, cust[ci]); used[(size_t)cust[ci]] = 1; ++ci; }
                    else break;
                }
            }
        }

        // ---------- 빈 라우트 제거 ----------
        void prune_empty_routes(std::vector<std::vector<int>>& routes) const {
            std::vector<std::vector<int>> keep; keep.reserve(routes.size());
            for (auto& r : routes) { if (r.size() <= 2 && r.front() == r.back()) continue; keep.emplace_back(std::move(r)); }
            routes.swap(keep);
        }

        // ---------- Δ 계산 헬퍼 (O(1) 간선 차분) ----------
        inline int64_t edge_cost(int x, int y, bool with_gls = true) const {
            int64_t c = dist(x, y);
            if (with_gls && !gls_pen_.empty()) c += (int64_t)(gls_lambda_ * gls_pen_[eid(x, y)]);

            // ★ 추가: 비후보 간선이면 약한 가산비 (LKH의 candidate set 효과 흉내)
            if (params_.enforce_candidate_bias) {
                bool is_cand = false;
                for (int v : cand_[x]) { if (v == y) { is_cand = true; break; } }
                if (!is_cand) c += (int64_t)(avg_edge_cost_ * params_.noncand_penalty_ratio);
            }
            return c;
        }

        inline int64_t delta_replace_edges(int a, int b, int c, int d, bool with_gls = true) const {
            // replace (a-b)+(c-d) -> (a-c)+(b-d)
            return edge_cost(a, c, with_gls) + edge_cost(b, d, with_gls) - (edge_cost(a, b, with_gls) + edge_cost(c, d, with_gls));
        }

        // ---------- 로컬서치 루프 ----------
        void local_search(std::vector<std::vector<int>>& R, int64_t& best, const RoutingSearchParameters& p) {
            for (int round = 0; round < p.ls_rounds; ++round) {
                bool imp = false;

                // intra
                for (int v = 0; v < (int)R.size(); ++v) {
                    if (p.use_oropt)  imp |= or_opt(R, v, best);
                    if (p.use_two_opt) imp |= two_opt(R, v, best, p.two_opt_span_limit);
                    if (p.use_three_opt) imp |= three_opt(R, v, best);
                    if (GetUseRelocate(p)) imp |= relocate_intra(R, v, best);
                }

                // inter
                if (GetUseRelocate(p))         imp |= relocate_inter(R, best);
                if (p.use_cross_exchange_k1)imp |= cross_exchange_k1(R, best);
                if (p.use_cross_exchange_k2)imp |= cross_exchange_k2(R, best); // ✅ k=2 일반화

                if (!imp) break;
            }
        }

        // ---------- Or-opt (len 1..3) ----------
        bool or_opt(std::vector<std::vector<int>>& R, int v, int64_t& best) {
            auto& r = R[v]; if (r.size() < 4) return false;
            bool imp = false;
            for (size_t len = 1; len <= 3 && len + 1 < r.size(); ++len) {
                for (size_t i = 1; i + len < r.size(); ++i) {
                    int x_prev = r[i - 1], x_first = r[i], x_last = r[i + len - 1], x_next = r[i + len];
                    // 제거 Δ: (x_prev-x_first)+(x_last-x_next) -> (x_prev-x_next)
                    int64_t delta_remove = edge_cost(x_prev, x_next) - (edge_cost(x_prev, x_first) + edge_cost(x_last, x_next));
                    for (size_t j = 1; j < r.size(); ++j) {
                        if (j >= i && j <= i + len) continue;
                        int y_prev = r[j - 1], y_next = r[j];
                        // 삽입 Δ: (y_prev-y_next) -> (y_prev-x_first)+(x_last-y_next)
                        int64_t delta_insert = (edge_cost(y_prev, x_first) + edge_cost(x_last, y_next)) - edge_cost(y_prev, y_next);
                        int64_t dlt = delta_remove + delta_insert;
                        if (dlt < 0) {
                            // apply
                            //auto base = r; base.erase(base.begin() + i, base.begin() + i + len);
                            //base.insert(base.begin() + j - (j > i ? len : 0), r.begin() + i, r.begin() + i + len);
                            //----------------------------------------------------------------------
                            // ---- 안전한 블록으로 교체 ----
                            auto base = r;

                            const size_t n = r.size();
                            const size_t i0 = static_cast<size_t>(i);
                            const size_t i1 = i0 + static_cast<size_t>(len); // exclusive, [i0, i1)

                            // 1) 옮길 구간을 미리 복사
                            std::vector<int> seg(r.begin() + static_cast<std::ptrdiff_t>(i0),
                                r.begin() + static_cast<std::ptrdiff_t>(i1));

                            // 2) base에서 해당 구간 제거
                            base.erase(base.begin() + static_cast<std::ptrdiff_t>(i0),
                                base.begin() + static_cast<std::ptrdiff_t>(i1)); // base.size() == n - len

                            // 3) '원본 r'에서의 삽입 기준 j를 'base' 기준으로 환산, j가 제거구간 뒤였다면 len만큼 앞으로 당겨진다.
                            size_t pos = static_cast<size_t>(j);
                            if (pos > i1) { // 원본에서 제거구간 뒤쪽이면 len 만큼 당김
                                pos -= (i1 - i0);
                            } else if (pos >= i0 && pos <= i1) { // 원본에서 제거구간 내부/경계면은 이미 continue로 스킵했겠지만, 혹시 몰라 방어적으로 i0로 맞춘다(제거구간의 시작 위치).                                
                                pos = i0;
                            }

                            // 4) 경계 클램프: [0, base.size()]
                            if (pos > base.size()) pos = base.size();

                            // 5) 삽입
                            base.insert(base.begin() + static_cast<std::ptrdiff_t>(pos), seg.begin(), seg.end());
                            //----------------------------------------------------------------------

                            Feas f = eval_route(v, base);
                            if (!f.feasible) { /*시간/용량 위반이면 스킵(soft면 패널티 검토 가능)*/ }
                            int64_t obj = total_replace(R, base, v);
                            if (obj < best) { r.swap(base); best = obj; imp = true; }
                        }
                    }
                }
            }
            return imp;
        }

        // ---------- 2-opt (O(1) Δ) ----------
        bool two_opt(std::vector<std::vector<int>>& R, int v, int64_t& best, int span_lim) {
            auto& r = R[v]; int m = (int)r.size(); if (m < 5) return false;
            bool imp = false;
            for (int i = 1; i + 2 < m; ++i) {
                int kmax = std::min(m - 2, i + span_lim);
                for (int k = i + 1; k <= kmax; ++k) {
                    int a = r[i - 1], b = r[i], c = r[k], d = r[k + 1];
                    int64_t dlt = delta_replace_edges(a, b, c, d, true);
                    if (dlt < 0) {
                        std::reverse(r.begin() + i, r.begin() + k + 1);
                        int64_t obj = total_replace(R, r, v);
                        if (obj < best) { best = obj; imp = true; }
                        else std::reverse(r.begin() + i, r.begin() + k + 1);
                    }
                }
            }
            return imp;
        }

        // ---------- 3-opt (7 패턴, O(1) Δ) ----------
        bool three_opt(std::vector<std::vector<int>>& R, int v, int64_t& best) {
            auto& T = R[v]; int m = (int)T.size(); if (m < 6) return false;
            bool imp = false;
            // break at (a,b),(c,d),(e,f)
            for (int a = 0; a < m - 5; ++a) for (int c = a + 2; c < m - 3; ++c) for (int e = c + 2; e < m - 1; ++e) {
                int A = T[a], B = T[a + 1], C = T[c], D = T[c + 1], E = T[e], F = T[e + 1];
                // 원래: A-B, C-D, E-F 연결
                // 패턴들: (예시 7종)
                struct P { int t; int64_t d; }; std::vector<P> cand;
                auto EC = [&](int x, int y) {return edge_cost(x, y, true); };

                // P1: reverse(B..C)   : replace (A-B)+(C-D) -> (A-C)+(B-D)
                cand.push_back({ 1, (EC(A,C) + EC(B,D)) - (EC(A,B) + EC(C,D)) });
                // P2: reverse(D..E)
                cand.push_back({ 2, (EC(C,E) + EC(D,F)) - (EC(C,D) + EC(E,F)) });
                // P3: reverse(B..E) (둘 다 뒤집기 성격)
                cand.push_back({ 3, (EC(A,E) + EC(B,F)) - (EC(A,B) + EC(E,F)) });
                // P4: reconnect A-D, C-F, E-B (링 재조합형, Δ는 세 간선 치환으로 구성)
                cand.push_back({ 4, (EC(A,D) + EC(C,F) + EC(E,B)) - (EC(A,B) + EC(C,D) + EC(E,F)) });
                // P5: A-D, C-B, E-F
                cand.push_back({ 5, (EC(A,D) + EC(C,B)) - (EC(A,B) + EC(C,D)) });
                // P6: A-B, C-F, E-D
                cand.push_back({ 6, (EC(C,F) + EC(E,D)) - (EC(C,D) + EC(E,F)) });
                // P7: A-E, D-B, C-F
                cand.push_back({ 7, (EC(A,E) + EC(D,B) + EC(C,F)) - (EC(A,B) + EC(C,D) + EC(E,F)) });

                for (auto& p : cand) {
                    if (p.d >= 0) continue;
                    auto r = T;
                    switch (p.t) {
                    case 1: std::reverse(r.begin() + a + 1, r.begin() + c + 1); break;
                    case 2: std::reverse(r.begin() + c + 1, r.begin() + e + 1); break;
                    case 3: std::reverse(r.begin() + a + 1, r.begin() + e + 1); break;
                    case 4: { // 재배열: A-(D..C+1)-F..E+1-B..a+1  (간단 구현: 두 구간 뒤집고 스플라이스)
                        std::reverse(r.begin() + a + 1, r.begin() + c + 1);
                        std::reverse(r.begin() + c + 1, r.begin() + e + 1);
                    } break;
                    case 5: { std::reverse(r.begin() + a + 1, r.begin() + c + 1); } break;
                    case 6: { std::reverse(r.begin() + c + 1, r.begin() + e + 1); } break;
                    case 7: {
                        std::reverse(r.begin() + a + 1, r.begin() + c + 1);
                        std::reverse(r.begin() + c + 1, r.begin() + e + 1);
                    } break;
                    }
                    int64_t obj = total_replace(R, r, v);
                    if (obj < best) { T.swap(r); best = obj; imp = true; }
                }
            }
            return imp;
        }

        // ---------- Relocate (intra/inter) ----------
        bool relocate_intra(std::vector<std::vector<int>>& R, int v, int64_t& best) {
            auto& r = R[v]; if (r.size() < 4) return false;
            bool imp = false;
            for (size_t i = 1; i + 1 < r.size(); ++i) {
                int node = r[i], a = r[i - 1], b = r[i], c = r[i + 1];
                // 제거 Δ: (a-b)+(b-c) -> (a-c)
                int64_t delta_remove = edge_cost(a, c) - (edge_cost(a, b) + edge_cost(b, c));
                auto base = r; base.erase(base.begin() + i);
                for (size_t j = 1; j < base.size(); ++j) {
                    int y_prev = base[j - 1], y_next = base[j];
                    // 삽입 Δ: (y_prev-y_next) -> (y_prev-b)+(b-y_next)
                    int64_t dlt = delta_remove + (edge_cost(y_prev, b) + edge_cost(b, y_next) - edge_cost(y_prev, y_next));
                    if (dlt < 0) {
                        auto cand = base; cand.insert(cand.begin() + j, node);
                        int64_t obj = total_replace(R, cand, v);
                        if (obj < best) { r.swap(cand); best = obj; imp = true; }
                    }
                }
            }
            return imp;
        }

        bool relocate_inter(std::vector<std::vector<int>>& R, int64_t& best) {
            bool imp = false;
            for (int v1 = 0; v1 < (int)R.size(); ++v1) for (int v2 = 0; v2 < (int)R.size(); ++v2) {
                if (v1 == v2) continue;
                auto& A = R[v1]; auto& B = R[v2];
                if (A.size() < 3) continue;
                for (size_t i = 1; i + 1 < A.size(); ++i) {
                    int x = A[i], a = A[i - 1], b = A[i], c = A[i + 1];
                    int64_t delta_remove = edge_cost(a, c) - (edge_cost(a, b) + edge_cost(b, c));
                    auto A1 = A; A1.erase(A1.begin() + i);
                    for (size_t j = 1; j < B.size(); ++j) {
                        int y_prev = B[j - 1], y_next = B[j];
                        int64_t dlt = delta_remove + (edge_cost(y_prev, b) + edge_cost(b, y_next) - edge_cost(y_prev, y_next));
                        if (dlt < 0) {
                            auto B1 = B; B1.insert(B1.begin() + j, x);
                            int64_t obj = total_replace_bi(R, A1, v1, B1, v2);
                            if (obj < best) { A.swap(A1); B.swap(B1); best = obj; imp = true; }
                        }
                    }
                }
            }
            return imp;
        }

        // ---------- Cross-exchange k=1 ----------
        bool cross_exchange_k1(std::vector<std::vector<int>>& R, int64_t& best) {
            bool imp = false;
            for (int v1 = 0; v1 < (int)R.size(); ++v1) for (int v2 = v1 + 1; v2 < (int)R.size(); ++v2) {
                auto& A = R[v1]; auto& B = R[v2];
                if (A.size() < 3 || B.size() < 3) continue;
                for (size_t i = 1; i + 1 < A.size(); ++i) for (size_t j = 1; j + 1 < B.size(); ++j) {
                    int a_prev = A[i - 1], x = A[i], a_next = A[i + 1];
                    int b_prev = B[j - 1], y = B[j], b_next = B[j + 1];
                    // Δ = 제거(A의 주변) + 제거(B의 주변) + 삽입(A에 y) + 삽입(B에 x)
                    int64_t d = 0;
                    d += (edge_cost(a_prev, a_next) - (edge_cost(a_prev, x) + edge_cost(x, a_next)));
                    d += (edge_cost(b_prev, b_next) - (edge_cost(b_prev, y) + edge_cost(y, b_next)));
                    d += (edge_cost(a_prev, y) + edge_cost(y, a_next) - edge_cost(a_prev, a_next));
                    d += (edge_cost(b_prev, x) + edge_cost(x, b_next) - edge_cost(b_prev, b_next));
                    if (d < 0) {
                        auto A1 = A, B1 = B; std::swap(A1[i], B1[j]);
                        int64_t obj = total_replace_bi(R, A1, v1, B1, v2);
                        if (obj < best) { A.swap(A1); B.swap(B1); best = obj; imp = true; }
                    }
                }
            }
            return imp;
        }

        // ---------- Cross-exchange k=2 (일반화, 연속 2노드 블록) ----------
        bool cross_exchange_k2(std::vector<std::vector<int>>& R, int64_t& best) {
            bool imp = false;
            for (int v1 = 0; v1 < (int)R.size(); ++v1) for (int v2 = v1 + 1; v2 < (int)R.size(); ++v2) {
                auto& A = R[v1]; auto& B = R[v2];
                if (A.size() < 4 || B.size() < 4) continue;
                for (size_t i = 1; i + 2 < A.size(); ++i) for (size_t j = 1; j + 2 < B.size(); ++j) {
                    // 블록: A[i],A[i+1] ↔ B[j],B[j+1]
                    // 패턴 4가지: (A정/B정), (A역/B정), (A정/B역), (A역/B역)
                    for (int pa = 0; pa < 2; ++pa) for (int pb = 0; pb < 2; ++pb) {
                        int A1_0 = A[i], A1_1 = A[i + 1]; if (pa) std::swap(A1_0, A1_1);
                        int B1_0 = B[j], B1_1 = B[j + 1]; if (pb) std::swap(B1_0, B1_1);

                        // Δ 계산: A에서 (i-1)-(i),(i+1)-(i+2) 제거 후 B블록 삽입, B도 반대
                        int a_prev = A[i - 1], a0 = A[i], a1 = A[i + 1], a_next = A[i + 2];
                        int b_prev = B[j - 1], b0 = B[j], b1 = B[j + 1], b_next = B[j + 2];

                        int64_t d = 0;
                        // A 제거 + B삽입
                        d += edge_cost(a_prev, a_next) - (edge_cost(a_prev, a0) + edge_cost(a1, a_next));
                        d += (edge_cost(a_prev, B1_0) + edge_cost(B1_1, a_next)) - edge_cost(a_prev, a_next);
                        // B 제거 + A삽입
                        d += edge_cost(b_prev, b_next) - (edge_cost(b_prev, b0) + edge_cost(b1, b_next));
                        d += (edge_cost(b_prev, A1_0) + edge_cost(A1_1, b_next)) - edge_cost(b_prev, b_next);

                        if (d < 0) {
                            auto A2 = A, B2 = B;
                            // 적용
                            A2.erase(A2.begin() + i, A2.begin() + i + 2);
                            A2.insert(A2.begin() + i, { B1_0, B1_1 });
                            B2.erase(B2.begin() + j, B2.begin() + j + 2);
                            B2.insert(B2.begin() + j, { A1_0, A1_1 });
                            int64_t obj = total_replace_bi(R, A2, v1, B2, v2);
                            if (obj < best) { A.swap(A2); B.swap(B2); best = obj; imp = true; }
                        }
                    }
                }
            }
            return imp;
        }

        // ---------- TW hard-mode Repair ----------
        void repair_time_windows(std::vector<std::vector<int>>& R, const RoutingSearchParameters& p) {
            for (int it = 0; it < p.repair_max_iters; ++it) {
                bool fixed = false;
                for (int v = 0; v < (int)R.size(); ++v) {
                    auto late_idx = find_late_indices(v, R[v]);
                    if (late_idx.empty()) continue;
                    for (int pos : late_idx) {
                        if (pos <= 0 || pos >= (int)R[v].size() - 1) continue;
                        int node = R[v][pos];
                        auto r0 = R[v]; r0.erase(r0.begin() + pos);

                        int bestV = -1, bestPos = -1; int64_t best = std::numeric_limits<int64_t>::max();
                        for (int w = 0; w < (int)R.size(); ++w) {
                            for (size_t j = 1; j < R[w].size(); ++j) {
                                auto cand = R[w]; cand.insert(cand.begin() + j, node);
                                Feas f = eval_route(w, cand);
                                if (f.late_pen == 0 && (!p.forbid_cap_violation || f.cap_violation == 0)) {
                                    int64_t obj = total_replace_bi(R, (w == v) ? r0 : R[v], v, cand, w);
                                    if (obj < best) { best = obj; bestV = w; bestPos = (int)j; }
                                }
                            }
                        }
                        if (bestV >= 0) {
                            if (bestV == v) { R[v].erase(R[v].begin() + pos); R[v].insert(R[v].begin() + bestPos, node); }
                            else { R[v].erase(R[v].begin() + pos); R[bestV].insert(R[bestV].begin() + bestPos, node); }
                            fixed = true;
                        }
                    }
                }
                if (!fixed) break;
            }
        }

        std::vector<int> find_late_indices(int v, const std::vector<int>& r) const {
            std::vector<int> idxs;
            int64_t t = t0_[v];
            for (size_t i = 0; i < r.size(); ++i) {
                int u = r[i];
                if (t < tw_early_[u]) t = tw_early_[u];
                if (t > tw_late_[u]) idxs.push_back((int)i);
                t += service_[u];
                if (i + 1 < r.size()) t += tcost(u, r[i + 1]);
            }
            return idxs;
        }

        // ---------- GLS ----------
        void guided_local_search(std::vector<std::vector<int>>& routes, int64_t& best, const RoutingSearchParameters& p) {
            if (gls_pen_.empty()) gls_pen_.assign((size_t)N_ * (size_t)N_, 0);
            gls_lambda_ = p.gls_lambda;
            for (int it = 0; it < p.gls_iters; ++it) {
                int bu = -1, bv = -1; int64_t bscore = -1;
                for (auto& r : routes) {
                    for (size_t i = 0; i + 1 < r.size(); ++i) {
                        int a = r[i], b = r[i + 1];
                        int64_t score = dist(a, b) + (int64_t)(gls_lambda_ * gls_pen_[eid(a, b)]);
                        if (score > bscore) { bscore = score; bu = a; bv = b; }
                    }
                }
                if (bu >= 0) gls_pen_[eid(bu, bv)] += 1;
                local_search(routes, best, p);
            }
        }

        // ---------- 비용 합치기 헬퍼 ----------
        int64_t total_replace(std::vector<std::vector<int>>& R,
            const std::vector<int>& cand, int v) const {
            auto bak = R[v]; R[v] = cand;
            int64_t c = total_cost(R, true);
            R[v] = bak; return c;
        }
        int64_t total_replace_bi(std::vector<std::vector<int>>& R,
            const std::vector<int>& A1, int v1,
            const std::vector<int>& B1, int v2) const {
            auto a = R[v1], b = R[v2]; R[v1] = A1; R[v2] = B1;
            int64_t c = total_cost(R, true);
            R[v1] = a; R[v2] = b; return c;
        }
        size_t aggregate_size(const std::vector<std::vector<int>>& R) const {
            size_t s = 0; for (auto& r : R) s += r.size(); return s;
        }

        // 멤버
        RoutingIndexManager& M_;
        int N_, V_;
        std::function<int64_t(int64_t, int64_t)> dist_, time_;
        std::vector<int64_t> demand_, tw_early_, tw_late_, service_;
        std::vector<int64_t> cap_, t0_;
        std::vector<std::vector<int>> cand_;
        std::vector<int> gls_pen_; double gls_lambda_ = 0.0;
        std::vector<Assignment> assigns_;
        RoutingSearchParameters params_; // SolveWithParameters에서 복사해 둘 저장소
        double avg_edge_cost_ = 0.0;     // 전체 평균 간선비 (스케일 기준)
    };


// ============================================================
// Endpoint helpers (FixedStart / FixedEnd / AutoStart variants)
// ============================================================
//
// These helpers mirror common OR-Tools patterns used in Python:
// - fixed_start        : start fixed at depot, end is "open" (dummy sink)
// - fixed_end          : end fixed at end_node, start is "open" (dummy source)
// - fixed_start_end    : start fixed at depot, end fixed at end_node
// - return_to_depot    : classic closed VRP/TSP (start==end==depot)
// - auto_start_*       : choose the best depot among candidates (random subset)
//
// NOTE
//  - For fixed_start / fixed_end we add one dummy node and build a manager with
//    starts/ends vectors (multi-vehicle supported).
//  - Routes returned are filtered to original node ids (dummy node removed).
//  - This file is a "mini" solver; auto_start splits the search effort by
//    scaling down ls_rounds/gls_iters per trial (no hard time-limit here).

enum class EndpointType {
    ReturnToDepot,
    FixedStart,        // start fixed, end open (dummy sink)
    FixedEnd,          // end fixed, start open (dummy source)
    FixedStartEnd,     // start fixed, end fixed
    AutoStartReturnToDepot,   // choose best depot, closed routes
    AutoStartFixedEnd,        // start/end both unspecified: choose best (start,end) open route
    RandomStartFixedEnd       // choose random depot once, end fixed
};

struct EndpointSolveMeta {
    int num_nodes_original = 0;
    int num_nodes_internal = 0;
    int sink_idx = -1;     // dummy sink index (if any)
    int source_idx = -1;   // dummy source index (if any)
    int depot = -1;        // chosen depot (for auto-start)
    int end_node = -1;     // fixed end (when applicable)
    int auto_trials = 0;
    int64_t objective = -1;
};

struct EndpointSolveResult {
    // 200: ok, 300: no-solution, 400: invalid input
    int resultCode = 300;
    EndpointType endpoint_type = EndpointType::ReturnToDepot;
    std::vector<std::vector<int>> routes; // per-vehicle node list (original ids)
    EndpointSolveMeta meta;
    std::string why; // filled only when resultCode==400
};

inline bool _is_square_matrix(const std::vector<std::vector<int64_t>>& m) {
    const int n = (int)m.size();
    for (int i = 0; i < n; ++i) if ((int)m[i].size() != n) return false;
    return true;
}

// Add dummy sink node at the end.
// - cost from any node -> sink = sink_cost
// - sink row (outgoing) is zeros by default (unused; solver may still read it)
inline std::vector<std::vector<int64_t>> AddSink(const std::vector<std::vector<int64_t>>& dm, int64_t sink_cost = 0) {
    const int n = (int)dm.size();
    std::vector<std::vector<int64_t>> out = dm;
    for (auto& row : out) row.push_back(sink_cost);
    out.push_back(std::vector<int64_t>(n + 1, 0));
    return out;
}

// Add dummy source node at the end.
// - cost from source -> any node = source_cost
// - cost from any node -> source = 0 (unused)
inline std::vector<std::vector<int64_t>> AddSource(const std::vector<std::vector<int64_t>>& dm, int64_t source_cost = 0) {
    const int n = (int)dm.size();
    std::vector<std::vector<int64_t>> out = dm;
    for (auto& row : out) row.push_back(0);
    out.push_back(std::vector<int64_t>(n + 1, source_cost));
    out.back()[n] = 0; // source->source
    return out;
}

inline std::vector<std::vector<int>> ExtractRoutesFiltered(
    RoutingIndexManager& manager,
    RoutingModel& routing,
    const Assignment* solution,
    int n_original,
    EndpointType endpoint_type,
    int depot,
    int sink_idx,
    int source_idx
) {
    std::vector<std::vector<int>> routes;
    routes.reserve(manager.num_vehicles());

    for (int v = 0; v < manager.num_vehicles(); ++v) {
        int64_t idx = routing.Start(v);
        std::vector<int> path;
        while (!routing.IsEnd(idx)) {
            const int node = manager.IndexToNode(idx).value();
            if (node >= 0 && node < n_original) path.push_back(node);
            idx = solution->Value(routing.NextVar(idx));
        }
        {
            const int node = manager.IndexToNode(idx).value();
            if (node >= 0 && node < n_original) path.push_back(node);
        }

        // For closed tours, the last node may repeat depot; drop that tail.
        if (endpoint_type == EndpointType::ReturnToDepot ||
            endpoint_type == EndpointType::AutoStartReturnToDepot ||
            endpoint_type == EndpointType::FixedStartEnd) {
            if ((int)path.size() >= 2 && path.back() == depot) path.pop_back();
        }
        routes.push_back(std::move(path));
    }
    return routes;
}

inline void _set_default_cb_cost(RoutingModel& routing, RoutingIndexManager& manager,
                                const std::vector<std::vector<int64_t>>& dm) {
    int64_t cb = routing.RegisterTransitCallback(std::function<int64_t(int64_t, int64_t)>(
        [&](int64_t a, int64_t b) {
            // a,b are internal node ids
            return dm[(int)a][(int)b];
        }
    ));
    routing.SetArcCostEvaluatorOfAllVehicles(cb);
}

inline EndpointSolveResult SolveVRPWithEndpoint(
    const std::vector<std::vector<int64_t>>& matrix,
    int num_vehicles,
    int depot,
    EndpointType endpoint_type,
    int end_node = -1,
    int auto_trials = 0,
    uint32_t rng_seed = 0,
    const RoutingSearchParameters& base_params = RoutingSearchParameters()
) {
    EndpointSolveResult out;
    out.endpoint_type = endpoint_type;
    out.meta.num_nodes_original = (int)matrix.size();
    out.meta.depot = depot;
    out.meta.end_node = end_node;
    out.meta.auto_trials = auto_trials;

    // -------- validate --------
    if (matrix.empty() || !_is_square_matrix(matrix)) {
        out.resultCode = 400;
        out.why = "matrix must be non-empty square matrix";
        return out;
    }
    const int n = (int)matrix.size();
    if (num_vehicles < 1 || num_vehicles > n) {
        out.resultCode = 400;
        out.why = "num_vehicles must be 1..n";
        return out;
    }
    if (depot < 0 || depot >= n) {
        out.resultCode = 400;
        out.why = "invalid depot index";
        return out;
    }
    if ((endpoint_type == EndpointType::FixedEnd ||
         endpoint_type == EndpointType::FixedStartEnd ||
         endpoint_type == EndpointType::AutoStartFixedEnd ||
         endpoint_type == EndpointType::RandomStartFixedEnd) &&
        (end_node < 0 || end_node >= n)) {
        out.resultCode = 400;
        out.why = "invalid end_node index";
        return out;
    }

    // -------- internal builders --------
    auto solve_once = [&](const std::vector<std::vector<int64_t>>& dm,
                         const std::vector<RoutingIndexManager::NodeIndex>& starts,
                         const std::vector<RoutingIndexManager::NodeIndex>& ends,
                         int used_depot,
                         int sink_idx,
                         int source_idx,
                         const RoutingSearchParameters& params)->EndpointSolveResult {

        EndpointSolveResult r;
        r.endpoint_type = endpoint_type;
        r.meta.num_nodes_original = n;
        r.meta.num_nodes_internal = (int)dm.size();
        r.meta.depot = used_depot;
        r.meta.end_node = end_node;
        r.meta.sink_idx = sink_idx;
        r.meta.source_idx = source_idx;
        r.meta.auto_trials = auto_trials;

        RoutingIndexManager manager((int)dm.size(), num_vehicles, starts, ends);
        RoutingModel routing(manager);
        _set_default_cb_cost(routing, manager, dm);

        RoutingSearchParameters p = params;
        const Assignment* sol = routing.SolveWithParameters(p);
        if (!sol) {
            r.resultCode = 300;
            return r;
        }

        r.resultCode = 200;
        r.meta.objective = sol->ObjectiveValue();
        r.routes = ExtractRoutesFiltered(manager, routing, sol, n, endpoint_type, used_depot, sink_idx, source_idx);
        return r;
    };

    auto solve_return_to_depot = [&](int used_depot, const RoutingSearchParameters& params)->EndpointSolveResult {
        std::vector<RoutingIndexManager::NodeIndex> starts(num_vehicles, RoutingIndexManager::NodeIndex(used_depot));
        std::vector<RoutingIndexManager::NodeIndex> ends(num_vehicles, RoutingIndexManager::NodeIndex(used_depot));
        return solve_once(matrix, starts, ends, used_depot, -1, -1, params);
    };

    auto solve_fixed_start = [&](int used_depot, const RoutingSearchParameters& params)->EndpointSolveResult {
        auto dm = AddSink(matrix, /*sink_cost=*/0);
        int sink_idx = (int)dm.size() - 1;
        std::vector<RoutingIndexManager::NodeIndex> starts(num_vehicles, RoutingIndexManager::NodeIndex(used_depot));
        std::vector<RoutingIndexManager::NodeIndex> ends(num_vehicles, RoutingIndexManager::NodeIndex(sink_idx));
        return solve_once(dm, starts, ends, used_depot, sink_idx, -1, params);
    };

    auto solve_fixed_end_open_start = [&](const RoutingSearchParameters& params)->EndpointSolveResult {
        auto dm = AddSource(matrix, /*source_cost=*/0);
        int source_idx = (int)dm.size() - 1;
        std::vector<RoutingIndexManager::NodeIndex> starts(num_vehicles, RoutingIndexManager::NodeIndex(source_idx));
        std::vector<RoutingIndexManager::NodeIndex> ends(num_vehicles, RoutingIndexManager::NodeIndex(end_node));
        return solve_once(dm, starts, ends, /*used_depot=*/depot, -1, source_idx, params);
    };

    auto solve_fixed_start_end = [&](int used_depot, const RoutingSearchParameters& params)->EndpointSolveResult {
        std::vector<RoutingIndexManager::NodeIndex> starts(num_vehicles, RoutingIndexManager::NodeIndex(used_depot));
        std::vector<RoutingIndexManager::NodeIndex> ends(num_vehicles, RoutingIndexManager::NodeIndex(end_node));
        return solve_once(matrix, starts, ends, used_depot, -1, -1, params);
    };

    // start/end pair (no dummy nodes). Used for AutoStartFixedEnd (start&end unspecified) search.
    auto solve_start_end_pair = [&](int start_node, int end_node2, const RoutingSearchParameters& params)->EndpointSolveResult {
        std::vector<RoutingIndexManager::NodeIndex> starts(num_vehicles, RoutingIndexManager::NodeIndex(start_node));
        std::vector<RoutingIndexManager::NodeIndex> ends(num_vehicles, RoutingIndexManager::NodeIndex(end_node2));
        EndpointSolveResult r = solve_once(matrix, starts, ends, start_node, -1, -1, params);
        r.meta.depot = start_node;
        r.meta.end_node = end_node2;
        return r;
    };


    // -------- dispatch --------
    if (endpoint_type == EndpointType::ReturnToDepot) {
        out = solve_return_to_depot(depot, base_params);
        out.endpoint_type = endpoint_type;
        return out;
    }
    if (endpoint_type == EndpointType::FixedStart) {
        out = solve_fixed_start(depot, base_params);
        out.endpoint_type = endpoint_type;
        return out;
    }
    if (endpoint_type == EndpointType::FixedEnd) {
        // 목적지 고정 + 출발지 미지정(오픈 시작): dummy source 사용
        out = solve_fixed_end_open_start(base_params);
        out.endpoint_type = endpoint_type;
        return out;
    }
    if (endpoint_type == EndpointType::FixedStartEnd) {
        out = solve_fixed_start_end(depot, base_params);
        out.endpoint_type = endpoint_type;
        return out;
    }

    // -------- random/best depot selection --------
    auto clamp_trials = [&](int t)->int {
        if (t <= 0) t = std::min(n, 16);
        t = std::max(1, std::min(t, n));
        return t;
    };

    const int trials = clamp_trials(auto_trials);
    std::vector<int> candidates(n);
    std::iota(candidates.begin(), candidates.end(), 0);

    std::mt19937 rng(rng_seed ? rng_seed : std::random_device{}());
    std::shuffle(candidates.begin(), candidates.end(), rng);
    candidates.resize(trials);

    // Scale down work per trial to keep total effort reasonable.
    auto per_params = base_params;
    per_params.ls_rounds = std::max(1, base_params.ls_rounds / trials);
    per_params.gls_iters = std::max(1, base_params.gls_iters / trials);

    if (endpoint_type == EndpointType::RandomStartFixedEnd) {
        const int chosen = candidates[0];
        out = solve_fixed_start_end(chosen, base_params);
        out.endpoint_type = endpoint_type;
        out.meta.auto_trials = trials;
        out.meta.depot = chosen;
        return out;
    }

    EndpointSolveResult best;
    best.resultCode = 300;

    if (endpoint_type == EndpointType::AutoStartFixedEnd) {
        // start/end 모두 미지정(오픈 경로): (start,end) 후보 쌍을 샘플링하여 objective 최솟값 선택
        const int64_t nn = (int64_t)n * (int64_t)n;

        int max_pairs = auto_trials > 0 ? auto_trials : (int)std::min<int64_t>(nn, 64);
        max_pairs = std::max(1, (int)std::min<int64_t>(nn, max_pairs));

        std::mt19937 rng(rng_seed ? rng_seed : std::random_device{}());
        std::vector<std::pair<int, int>> sample_pairs;
        sample_pairs.reserve((size_t)max_pairs);

        if (nn <= max_pairs) {
            for (int s = 0; s < n; ++s) {
                for (int e = 0; e < n; ++e) {
                    sample_pairs.emplace_back(s, e); // s==e 허용(폐회로도 후보에 포함)
                }
            }
        } else {
            // 랜덤 샘플링 (중복 허용: 구현/호환성 단순화)
            for (int k = 0; k < max_pairs; ++k) {
                int s = (int)(rng() % (uint32_t)n);
                int e = (int)(rng() % (uint32_t)n);
                sample_pairs.emplace_back(s, e);
            }
        }

        // Scale down work per pair to keep total effort reasonable.
        auto per_pair_params = base_params;
        per_pair_params.ls_rounds = std::max(1, base_params.ls_rounds / std::max(1,(int)sample_pairs.size()));
        per_pair_params.gls_iters = std::max(1, base_params.gls_iters / std::max(1,(int)sample_pairs.size()));

        for (const auto& pr : sample_pairs) {
            EndpointSolveResult r = solve_start_end_pair(pr.first, pr.second, per_pair_params);
            if (r.resultCode != 200) continue;
            if (best.resultCode != 200 || r.meta.objective < best.meta.objective) best = std::move(r);
        }

        if (best.resultCode == 200) best.meta.auto_trials = (int)sample_pairs.size();
    } else {
        for (int cand : candidates) {
            EndpointSolveResult r;
            if (endpoint_type == EndpointType::AutoStartReturnToDepot) {
                r = solve_return_to_depot(cand, per_params);
            } else {
                // unknown endpoint_type
                continue;
            }

            if (r.resultCode != 200) continue;
            if (best.resultCode != 200 || r.meta.objective < best.meta.objective) best = std::move(r);
        }
    }

    if (best.resultCode != 200) {
        out.resultCode = 300;
        out.meta.num_nodes_internal = (int)matrix.size();
        out.meta.sink_idx = -1;
        out.meta.source_idx = -1;
        out.meta.depot = -1;
        out.meta.auto_trials = trials;
        out.endpoint_type = endpoint_type;
        return out;
    }

    best.endpoint_type = endpoint_type;
    best.meta.auto_trials = trials;
    return best;
}


} // namespace operations_research


#if 0 //사용법

#include "gn_modules/mini_ortools_mv_pro.hpp"
// using namespace operations_research;


// 1) 차량 1대, 출발지만 지정(= 폐회로 TSP : start == end)
// 의미: 한 대가 depot에서 출발해서 전 고객을 방문하고 다시 같은 depot으로 돌아오는 TSP.
// 왜 예제에{ 0 }, { 4 }가 있었나 ? → 그건 차량 2대일 때 각 차량의 depot을 0과 4로 따로 준 예시였어.
// 차량 1대면 하나만 지정하면 됨.


// using namespace operations_research;

// int n = (int)dm.size();
// RoutingIndexManager::NodeIndex depot(0);  // 출발지(=도착지)

// RoutingIndexManager M(n, /*num_vehicles=*/1, depot);
// RoutingModel R(M);

// 비용 콜백
// R.RegisterTransitCallback(
    // std::function<int64_t(int64_t, int64_t)>(
        // [&](int64_t a, int64_t b) { return dm[(int)a][(int)b]; }
    // )
// );

// (TSP라면 용량/수요/시간창 안 써도 OK)
// RoutingSearchParameters p;
// p.use_gls = true; p.use_two_opt = true; p.use_three_opt = true;
// 필요에 따라 ON/OFF

// const Assignment* sol = R.SolveWithParameters(p);

// 이 경우 노드 0이 “시작이자 끝”이야.
// DM의 다른 인덱스(1..n - 1)는 고객으로 자동 취급됨.



// 2) 차량 1대, 출발지와 도착지 지정(= 오픈 TSP : start != end 가능)
// 의미: 한 대가 start에서 출발해서 전 고객을 방문하고 다른 end 노드에서 종료.

// 방법 : 3 - 인자 생성자가 아니라 4 - 인자 생성자(starts / ends 벡터) 사용.

// int start_id = 0;   // 출발지
// int end_id = 4;   // 도착지 (start와 달라도 됨)

// std::vector<RoutingIndexManager::NodeIndex> starts{ RoutingIndexManager::NodeIndex(start_id) };
// std::vector<RoutingIndexManager::NodeIndex> ends{ RoutingIndexManager::NodeIndex(end_id) };

// RoutingIndexManager M(n, /*num_vehicles=*/1, starts, ends);
// RoutingModel R(M);

// R.RegisterTransitCallback(
    // std::function<int64_t(int64_t, int64_t)>(
        // [&](int64_t a, int64_t b) { return dm[(int)a][(int)b]; }
    // )
// );

// RoutingSearchParameters p;
// …
// const Assignment* sol = R.SolveWithParameters(p);

// start == end로 주면 1)과 같은 폐회로가 되고, start != end로 주면 오픈 경로가 됨.



// 3) SetVehicleCapacity(v, cap) 가 뭔가요 ?

// **VRP(용량 제약) * *에서 * *차량 v가 실을 수 있는 총량(용량) * *을 지정하는 함수.
// 기본값(아무 것도 안 부르면) : 매우 큰 수(사실상 무한) → 용량 제약이 없다시피 함.

// 예시 :
    // R.SetVehicleCapacity(0, 3); // 차량 0은 총 수요합 3까지 가능
// R.SetVehicleCapacity(1, 3); // 차량 1도 제한하려면 이렇게

// 당신 코드에서 1번 차량만 제한하고 2번 차량은 주석 처리하면,
// 차량 0만 용량 제약 3, 차량 1은 무제한이 돼요. (의도 확인 필요)

// 4) SetNodeDemand(node, demand) 가 뭔가요 ?
// 각 고객 노드가 요구하는 수요량(예 : 상차량 / 배송량)을 지정.
// 차량 라우트에서 방문 순으로 누적합이 해당 차량의 용량을 넘지 않도록 강제돼요.
// TSP(순수 경로최적화)만 할 거면 수요를 안 넣거나 0으로 두면 됨.

// 예시 :

    // R.SetNodeDemand(1, 1); // 고객 1의 수요 1
// R.SetNodeDemand(2, 1); // 고객 2의 수요 1
// … 총합이 vehicle capacity를 넘으면 해당 라우트는 불가능

// 보너스: 어떤 노드가 고객 / 터미널로 취급되나 ?

// RoutingIndexManager에 넘긴 starts / ends의 인덱스들은 “터미널”(차량 출발·종료 노드).
// 그 외 인덱스는 고객 노드로 자동 인식돼요.
// 예 : n = 6, starts = { 0 }, ends = { 0 } → 0은 터미널, 1~5가 고객.

// 실전 팁

// TSP(1대, 폐회로) : RoutingIndexManager(n, 1, NodeIndex(depot)) 만으로 충분.수요 / 용량 / 시간창 안 써도 됨.

// 오픈 TSP(1대) : starts / ends 벡터로 각각 1개씩 넣고 start != end 가능.
// VRP(여러 대) : starts / ends를 차량 수만큼 넣으세요.예 : 2대, depot가 0과 4라면

// starts = { NodeIndex(0), NodeIndex(4) }, ends = { NodeIndex(0), NodeIndex(4) }.

// 시간창을 강제하고 싶으면 p.allow_soft_tw = false; (지각 노드를 자동 수리 시도).

// double 거리면 콜백에서 스케일링해 int64_t로 넘기기(앞서 설명).
// 원하는 시나리오(예: “1대, 오픈, depot = 2→end = 5, TW 엄수, 수요 / 용량 포함”)를 말해주면, 그 설정 그대로 돌아가는 완성 코드 블록으로 바로 맞춰 드릴게요.

#endif