import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:csv/csv.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:intl/intl.dart';

class LiveStats extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _LiveStatsState();
}

class _LiveStatsState extends State<LiveStats> {
  bool refresh = false;

  var d = {};
  var queryType = 0;

  var liveStatus = [];

  var lastMidnight = DateTime(DateTime
      .now()
      .year, DateTime
      .now()
      .month, DateTime
      .now()
      .day);
  var lastLastMidnight = DateTime(DateTime
      .now()
      .subtract(Duration(days: 1))
      .year, DateTime
      .now()
      .subtract(Duration(days: 1))
      .month, DateTime
      .now()
      .subtract(Duration(days: 1))
      .day);

  List<FlSpot> timeList = [];

  LineChartData get sampleData2 =>
      LineChartData(
        lineTouchData: lineTouchData2,
        gridData: gridData,
        titlesData: titlesData2,
        borderData: borderData,
        lineBarsData: lineBarsData2,
        minX: 0,
        maxX: 24,
        maxY: 16,
        minY: 0,
      );

  LineTouchData get lineTouchData2 =>
      LineTouchData(
        enabled: true,
      );

  void initState() {
    super.initState();
    influxSetup();
    fetchLiveFirebase();
  }

  FlTitlesData get titlesData2 =>
      FlTitlesData(
        bottomTitles: bottomTitles,
        rightTitles: SideTitles(showTitles: false),
        topTitles: SideTitles(showTitles: false),
        leftTitles: leftTitles(
          getTitles: (value) {
            switch (value.toInt()) {
              case 1:
                return '100W';
              case 5:
                return '500W';
              case 10:
                return '1000W';
              case 15:
                return '1500W';
            }
            return '';
          },
        ),
      );

  List<LineChartBarData> get lineBarsData2 =>
      [
        lineChartBarData2_3,
      ];

  SideTitles leftTitles({required GetTitleFunction getTitles}) =>
      SideTitles(
        getTitles: getTitles,
        showTitles: true,
        margin: 8,
        interval: 1,
        reservedSize: 40,
        getTextStyles: (context, value) =>
        const TextStyle(
          color: Color(0xff75729e),
          fontWeight: FontWeight.bold,
          fontSize: 14,
        ),
      );

  SideTitles get bottomTitles =>
      SideTitles(
        showTitles: true,
        reservedSize: 22,
        margin: 10,
        interval: 1,
        getTextStyles: (context, value) =>
        const TextStyle(
          color: Color(0xff72719b),
          fontWeight: FontWeight.bold,
          fontSize: 16,
        ),
        getTitles: (value) {
          switch (value.toInt()) {
            case 1:
              return '${DateFormat('hh:mm a').format(d.keys.elementAt(0))}';
            case 12:
              return '${DateFormat('hh:mm a').format(
                  d.keys.elementAt((d.keys.length / 2).toInt()))}';
            case 24:
              return '${DateFormat('hh:mm a').format(
                  d.keys.elementAt(d.keys.length - 1))}';
          }
          return '';
        },
      );

  FlGridData get gridData => FlGridData(show: false);

  FlBorderData get borderData =>
      FlBorderData(
        show: true,
        border: const Border(
          bottom: BorderSide(color: Color(0xff4e4965), width: 4),
          left: BorderSide(color: Colors.transparent),
          right: BorderSide(color: Colors.transparent),
          top: BorderSide(color: Colors.transparent),
        ),
      );

  LineChartBarData get lineChartBarData2_3 =>
      LineChartBarData(
        isCurved: true,
        curveSmoothness: 0,
        colors: [Colors.blue],
        barWidth: 2,
        isStrokeCapRound: true,
        dotData: FlDotData(show: true),
        belowBarData: BarAreaData(show: false),
        spots: timeList, //d.map((item)=> FlSpot(3, item/100)).toList(),
        // [
        //   FlSpot(1, 3.8),
        //   FlSpot(3, 1.9),
        //   FlSpot(6, 5),
        //   FlSpot(10, 3.3),
        //   FlSpot(13, 4.5),
        // ],
      );

  void influxSetup() async {
    setState(() {
      d = {};
      timeList = [];
    });


    var headers = {
      'Authorization':
      'Token [TOKEN]',
      'Accept': 'application/csv',
      'Content-type': 'application/vnd.flux'
    };
    var request = http.Request(
        'POST',
        Uri.parse(
            'https://us-central1-1.gcp.cloud2.influxdata.com/api/v2/query?org=d7a63bfb9d1aa738'));
    print("Pls" + lastMidnight.toString());
    print(
        "last mifnight was: " + lastMidnight.toString().substring(0, 10) + "T" +
            lastMidnight.toString().substring(11));

    var bod =
    '''from(bucket: "sme-akash")\n |> range(start: v.timeRangeStart, stop: v.timeRangeStop)\n |> filter(fn: (r) => r["_measurement"] == "Phase1")\n |> filter(fn: (r) => r["_field"] == "power")\n |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)\n |> yield(name: "mean")''';
    if (queryType == 0) {
      request.body =
      '''import "date"\n\nfrom(bucket: "sme-akash")\n |> range(start: ${lastMidnight
          .toString().substring(0, 10) + "T" +
          lastMidnight.toString().substring(11) +
          "Z"}, stop: now())\n |> filter(fn: (r) => r["_measurement"] == "Phase1")\n |> filter(fn: (r) => r["_field"] == "power")\n |> aggregateWindow(every: 1h, fn: mean, createEmpty: false)\n |> yield(name: "mean")'''; //'''from(bucket: "sme-siva")\n  |> range(start: -1h, stop: -1m)''';
    } else if (queryType == 1) {
      request.body =
      '''import "date"\n\nfrom(bucket: "sme-akash")\n |> range(start: ${lastLastMidnight
          .toString().substring(0, 10) + "T" +
          lastLastMidnight.toString().substring(11) + "Z"}, stop: ${lastMidnight
          .toString().substring(0, 10) + "T" +
          lastMidnight.toString().substring(11) +
          "Z"})\n |> filter(fn: (r) => r["_measurement"] == "Phase1")\n |> filter(fn: (r) => r["_field"] == "power")\n |> aggregateWindow(every: 1h, fn: mean, createEmpty: false)\n |> yield(name: "mean")'''; //'''from(bucket: "sme-siva")\n  |> range(start: -1h, stop: -1m)''';
    }
    request.headers.addAll(headers);

    var response = await request.send();
    if (response.statusCode == 200) {
      var op = await http.Response.fromStream(response);
      List<List<dynamic>> tmp = CsvToListConverter().convert(op.body);
      tmp.removeAt(0);
      for (var i in tmp) {
        var z = 0;
        var dt = '';
        for (var t in i) {
          if (z == 5) {
            dt = t.toString();
            z = 0;
            continue;
          }
          if (t is double) {
            debugPrint("Val: " + t.toString() + " w date: " + dt);
            d[DateTime.parse(dt.substring(0, dt.length - 1)).add(
                Duration(hours: 5))] = t;
          }
          z += 1;
        }
      }
      var minVal = d.keys
          .elementAt(0)
          .hour
          .toDouble();
      print("Complete");
      setState(() {
        d.forEach((key, value) {
          if (queryType == 0) {
            timeList.add(FlSpot(
                key.hour.toDouble(), value / 100.0));
          } else {
            if (key.hour.toDouble() != 0.0) {
              timeList.add(FlSpot(
                  key
                      .hour.toDouble(), value / 100.0));
            }
          }
        });
      });
      debugPrint(d.toString());
      debugPrint(timeList.toString());
    } else {
      print(lastMidnight.toString());
      print(lastMidnight.toString().substring(0, 10) + "T" +
          lastMidnight.toString().substring(11));
      print(DateTime
          .now()
          .millisecondsSinceEpoch / 1000);
      print(response.reasonPhrase);
    }
  }

  void fetchLiveFirebase() async {
    // Get docs from collection reference
    FirebaseFirestore firestore = FirebaseFirestore.instance;

    CollectionReference status = firestore.collection('status');


    QuerySnapshot querySnapshot = await status.get();

    // Get data from docs and convert map to List
    setState(() {
      liveStatus = querySnapshot.docs.map((doc) =>
          doc.data()).toList();
    });


    print("Live data is:" + liveStatus.toString());
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
        child: Column(
          children: [
            Row(
              children: [
                IconButton(
                    onPressed: () {
                      queryType = 0;
                      influxSetup();
                    },
                    icon: Icon(Icons.refresh)),
                TextButton(
                    onPressed: () {
                      queryType = 1;
                      influxSetup();
                    },
                    child: Text("Previous Day"))
              ],
            ),
            Padding(
              padding: const EdgeInsets.only(right: 18.0, left: 12.0, top: 24),
              child: Text("Power use today",
                  style: const TextStyle(fontWeight: FontWeight.bold)),
            ),
            Stack(
              children: <Widget>[
                AspectRatio(
                  aspectRatio: 1.70,
                  child: Container(
                    decoration: const BoxDecoration(
                        borderRadius: BorderRadius.all(
                          Radius.circular(18),
                        ),
                        color: Color(0xffffff)),
                    child: Padding(
                      padding: const EdgeInsets.only(
                          right: 18.0, left: 12.0, top: 24, bottom: 12),
                      child: LineChart(
                        sampleData2,
                        swapAnimationDuration: const Duration(
                            milliseconds: 250),
                      ),
                    ),
                  ),
                ),
              ],
            ),
            Card(
                margin: EdgeInsets.all(10),
                // height: 200,
                // padding: EdgeInsets.all(20),
                color: Colors.white70,
                child: Padding(
                  padding: EdgeInsets.all(30),
                  child: Column(
                    children: [
                      Text(queryType == 0
                          ? "24 hour power consumption"
                          : "Yesterday's power consumption",
                          style: const TextStyle(fontWeight: FontWeight.bold)),
                      SizedBox(height: 10,),
                      Text(
                        "Total power consumption : " +
                            (d.length > 0 ? (d.values.reduce((value,
                                element) => value + element) / 1000)
                                .toStringAsFixed(2) : '...') + ' KW',
                        style: const TextStyle(color: Colors.black54),
                        textAlign: TextAlign.center,
                      ),
                    ],
                  ),
                )),

            Row(
              children: [
                SizedBox(
                  width: MediaQuery
                      .of(context)
                      .size
                      .width/2.1,
                  child:  Card(
                      margin: EdgeInsets.all(10),
                      // height: 200
                      // padding: EdgeInsets.all(20),
                      color: Colors.white70,
                      child: Padding(
                        padding: EdgeInsets.all(5),
                        child: Column(
                          children: [
                            Text("AC Status: ${liveStatus[0]['ac_status']}",
                                textAlign: TextAlign.center,
                                style: const TextStyle(
                                    fontWeight: FontWeight.bold)),
                            SizedBox(height: 10,),
                            Text(
                              "Last checked: ${ DateFormat('yyyy-MM-dd hh:mm')
                                  .format(
                                  liveStatus[0]['last_updated'].toDate())}",
                              style: const TextStyle(color: Colors.black54),
                              textAlign: TextAlign.center,
                            ),
                          ],
                        ),
                      )),
                )
               ,SizedBox(
                  width: MediaQuery
                      .of(context)
                      .size
                      .width/2.1,
                  child:  Card(
                      margin: EdgeInsets.all(10),
                      color: Colors.white70,
                      child: Padding(
                        padding: EdgeInsets.all(5),
                        child: Column(
                          children: [
                            Text("Geyser Status: ${liveStatus[1]['geyser_status']}",
                                textAlign: TextAlign.center,
                                style: const TextStyle(
                                    fontWeight: FontWeight.bold)),
                            SizedBox(height: 10,),
                            Text(
                              "Last checked: ${ DateFormat('yyyy-MM-dd hh:mm')
                                  .format(
                                  liveStatus[1]['last_updated'].toDate())}",
                              style: const TextStyle(color: Colors.black54),
                              textAlign: TextAlign.center,
                            ),
                          ],
                        ),
                      )),
                )
                ,
              ],
            )

            // ListView(
            //   padding: const EdgeInsets.all(8),
            //   shrinkWrap: true,
            //   physics: NeverScrollableScrollPhysics(),
            //   children: <Widget>[
            //     Card(
            //         margin: EdgeInsets.all(10),
            //         // height: 200,
            //         // padding: EdgeInsets.all(20),
            //         color: Colors.white70,
            //         child: Padding(
            //           padding: EdgeInsets.all(10),
            //           child: Column(
            //             children: [
            //               Text("01-10-21",
            //                   style: const TextStyle(fontWeight: FontWeight.bold)),
            //               Text(
            //                 "Total power consumption : 5 KW",
            //                 style: const TextStyle(color: Colors.black54),
            //                 textAlign: TextAlign.center,
            //               ),
            //             ],
            //           ),
            //         )),
            //     Card(
            //         margin: EdgeInsets.all(10),
            //         // height: 200,
            //         // padding: EdgeInsets.all(20),
            //         color: Colors.white70,
            //         child: Padding(
            //           padding: EdgeInsets.all(10),
            //           child: Column(
            //             children: [
            //               Text("02-10-21",
            //                   style: const TextStyle(fontWeight: FontWeight.bold)),
            //               Text(
            //                 "Total power consumption : 8 KW",
            //                 style: const TextStyle(color: Colors.black54),
            //                 textAlign: TextAlign.center,
            //               ),
            //             ],
            //           ),
            //         )),
            //     Card(
            //         margin: EdgeInsets.all(10),
            //         // height: 200,
            //         // padding: EdgeInsets.all(20),
            //         color: Colors.white70,
            //         child: Padding(
            //           padding: EdgeInsets.all(10),
            //           child: Column(
            //             children: [
            //               Text("03-10-21",
            //                   style: const TextStyle(fontWeight: FontWeight.bold)),
            //               Text(
            //                 "Total power consumption : 3 KW",
            //                 style: const TextStyle(color: Colors.black54),
            //                 textAlign: TextAlign.center,
            //               ),
            //             ],
            //           ),
            //         )),
            //   ],
            // )
          ],
        ));
  }
}
