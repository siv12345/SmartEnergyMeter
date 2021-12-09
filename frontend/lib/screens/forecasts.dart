import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:csv/csv.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter_rounded_progress_bar/flutter_rounded_progress_bar.dart';
import 'package:flutter_rounded_progress_bar/rounded_progress_bar_style.dart';
import 'package:intl/intl.dart';
import 'package:http/http.dart' as http;

class Forecasts extends StatefulWidget {
  @override
  _ForecastsState createState() => _ForecastsState();
}

class _ForecastsState extends State<Forecasts> {
  List<Color> gradientColors = [
    const Color(0xff23b6e6),
    const Color(0xff02d39a),
  ];

  var maxConsumption = 1200;
  var d = {};
  var predictionList = [];
  var lastMidnight =
      DateTime(DateTime.now().year, DateTime.now().month, DateTime.now().day);

  DateTime firstDayCurrentMonth =
      DateTime.utc(DateTime.now().year, DateTime.now().month, 1);

  DateTime lastDayCurrentMonth = DateTime.utc(
    DateTime.now().year,
    DateTime.now().month + 1,
  ).subtract(Duration(days: 1));

  List<FlSpot> timeList = [];
  List consumptionList = [];

  //var totalConsumption = 0.0;

  LineChartData get sampleData2 => LineChartData(
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

  LineTouchData get lineTouchData2 => LineTouchData(
        enabled: true,
      );

  FlTitlesData get titlesData2 => FlTitlesData(
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
              case 20:
                return '2000W';
            }
            return '';
          },
        ),
      );

  List<LineChartBarData> get lineBarsData2 => [
        lineChartBarData2_1,
        //lineChartBarData2_2,
        //lineChartBarData2_3,
      ];

  SideTitles leftTitles({required GetTitleFunction getTitles}) => SideTitles(
        getTitles: getTitles,
        showTitles: true,
        margin: 8,
        interval: 1,
        reservedSize: 40,
        getTextStyles: (context, value) => const TextStyle(
          color: Color(0xff75729e),
          fontWeight: FontWeight.bold,
          fontSize: 14,
        ),
      );

  SideTitles get bottomTitles => SideTitles(
        showTitles: true,
        reservedSize: 22,
        margin: 10,
        interval: 1,
        getTextStyles: (context, value) => const TextStyle(
          color: Color(0xff72719b),
          fontWeight: FontWeight.bold,
          fontSize: 16,
        ),
        getTitles: (value) {
          switch (value.toInt()) {
            case 1:
              return '${DateFormat('hh:mm a').format(d.keys.elementAt(0))}';
            case 12:
              return '${DateFormat('hh:mm a').format(d.keys.elementAt((d.keys.length / 2).toInt()))}';
            case 24:
              return '${DateFormat('hh:mm a').format(d.keys.elementAt(d.keys.length - 1))}';
          }
          return '';
        },
      );

  FlGridData get gridData => FlGridData(show: false);

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

    request.body =
        '''from(bucket: "sme-akash")\n |> range(start: ${lastMidnight.toString().substring(0, 10) + "T" + lastMidnight.toString().substring(11) + "Z"}, stop: now())\n |> filter(fn: (r) => r["_measurement"] == "Solar")\n |> filter(fn: (r) => r["_field"] == "power")\n |> aggregateWindow(every: 1h, fn: mean, createEmpty: false)\n |> yield(name: "mean")'''; //'''from(bucket: "sme-siva")\n  |> range(start: -1h, stop: -1m)''';

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
            d[DateTime.parse(dt.substring(0, dt.length - 1)).add(Duration(hours: 5))] = t;
          }
          z += 1;
        }
      }
      print("Completed solar");
      setState(() {
        d.forEach((key, value) {
          print("Val is : " + (value / 100).toString());
          timeList.add(FlSpot(key.hour.toDouble(), value / 100.0));
        });
      });
      debugPrint(d.toString());
      debugPrint(timeList.toString());
    } else {
      print(response.reasonPhrase);
    }
  }


  Future<void> getData() async {
    // Get docs from collection reference
    FirebaseFirestore firestore = FirebaseFirestore.instance;

    CollectionReference alerts = firestore.collection('predictions');


    QuerySnapshot querySnapshot = await alerts.get();

    // Get data from docs and convert map to List
    setState(() {
      predictionList = querySnapshot.docs.map((doc) =>
          doc.data()).toList();

    });


    print(predictionList);
  }

  // void totalPowerConsumption() async {
  //   var headers = {
  //     'Authorization':
  //         'Token [TOKEN]',
  //     'Accept': 'application/csv',
  //     'Content-type': 'application/vnd.flux'
  //   };
  //   var request = http.Request(
  //       'POST',
  //       Uri.parse(
  //           'https://us-central1-1.gcp.cloud2.influxdata.com/api/v2/query?org=d7a63bfb9d1aa738'));
  //
  //   request.body =
  //       '''import "date"\n\nmonth = date.truncate(t: now(), unit: 1mo)\nfrom(bucket: "sme-akash")\n  |> range(start: month)\n  |> filter(fn: (r) => r["_field"] == "energy")\n  |> spread()\n  |> yield(name: "spread")''';
  //   request.headers.addAll(headers);
  //
  //   var response = await request.send();
  //
  //   if (response.statusCode == 200) {
  //     var op = await http.Response.fromStream(response);
  //     debugPrint(op.body);
  //     List<List<dynamic>> tmp = CsvToListConverter().convert(op.body);
  //     print(tmp);
  //
  //     setState(() {
  //       totalConsumption = tmp[1][7] + tmp[2][7] + tmp[3][7];
  //     });
  //     print(totalConsumption);
  //   } else {
  //     print(response.reasonPhrase);
  //   }
  // }

  FlBorderData get borderData => FlBorderData(
        show: true,
        border: const Border(
          bottom: BorderSide(color: Color(0xff4e4965), width: 4),
          left: BorderSide(color: Colors.transparent),
          right: BorderSide(color: Colors.transparent),
          top: BorderSide(color: Colors.transparent),
        ),
      );

  LineChartBarData get lineChartBarData2_1 => LineChartBarData(
        isCurved: true,
        curveSmoothness: 0,
        colors: const [Color(0x444af699)],
        barWidth: 4,
        isStrokeCapRound: true,
        dotData: FlDotData(show: false),
        belowBarData: BarAreaData(show: false),
        spots: timeList,
        // [
        //   FlSpot(1, 1),
        //   FlSpot(3, 2),
        //   FlSpot(5, 5),
        //   FlSpot(7, 4),
        //   FlSpot(10, 3),
        //   FlSpot(12, 2),
        //   FlSpot(13, 1),
        // ],
      );

  // LineChartBarData get lineChartBarData2_2 => LineChartBarData(
  //       isCurved: true,
  //       colors: const [Color(0x99aa4cfc)],
  //       barWidth: 4,
  //       isStrokeCapRound: true,
  //       dotData: FlDotData(show: false),
  //       belowBarData: BarAreaData(
  //         show: true,
  //         colors: [
  //           const Color(0x33aa4cfc),
  //         ],
  //       ),
  //       spots: [
  //         FlSpot(1, 1),
  //         FlSpot(3, 2.8),
  //         FlSpot(7, 1.2),
  //         FlSpot(10, 2.8),
  //         FlSpot(12, 2.6),
  //         FlSpot(13, 3.9),
  //       ],
  //     );

  // LineChartBarData get lineChartBarData2_3 => LineChartBarData(
  //       isCurved: true,
  //       curveSmoothness: 0,
  //       colors: const [Color(0x4427b6fc)],
  //       barWidth: 2,
  //       isStrokeCapRound: true,
  //       dotData: FlDotData(show: true),
  //       belowBarData: BarAreaData(show: false),
  //       spots: [
  //         FlSpot(1, 1.4),
  //         FlSpot(3, 1.9),
  //         FlSpot(6, 6),
  //         FlSpot(10, 4),
  //         FlSpot(13, 2),
  //       ],
  //     );

  void initState() {
    super.initState();
    influxSetup();
    getData();
    //totalPowerConsumption();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: [
          Padding(
            padding: const EdgeInsets.only(right: 18.0, left: 18.0, top: 24),
            child: Text("Expected Power consumption ",
                style: const TextStyle(fontWeight: FontWeight.bold)),
          ),
          Padding(
              padding: EdgeInsets.all(18),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    "Energy usage ",
                    textAlign: TextAlign.left,
                    style: const TextStyle(color: Colors.black54),
                  ),
                  RoundedProgressBar(
                    childLeft: Text(
                        predictionList[0]['current_month_consumption'].toString()+ " KwH used",
                        style: const TextStyle(fontWeight: FontWeight.bold)),
                    style: RoundedProgressBarStyle(
                        borderWidth: 5,
                        colorBorder: Color.fromARGB(255, 32, 218, 162),
                        widthShadow: 0,
                        colorProgress: Colors.white,
                        backgroundProgress: Color.fromARGB(255, 32, 218, 162)),
                    margin: EdgeInsets.symmetric(vertical: 16),
                    borderRadius: BorderRadius.circular(12),
                    percent: (predictionList[0]['current_month_consumption'] / maxConsumption) * 100,
                  ),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Text(
                          DateFormat('MMMM').format(firstDayCurrentMonth) +
                              ' ' +
                              firstDayCurrentMonth.day.toString(),
                          style: const TextStyle(fontWeight: FontWeight.bold)),
                      Text(
                          DateFormat('MMMM').format(lastDayCurrentMonth) +
                              ' ' +
                              lastDayCurrentMonth.day.toString(),
                          style: const TextStyle(fontWeight: FontWeight.bold))
                    ],
                  ),
                  SizedBox(
                    height: 10,
                  ),
                  Center(
                      child: Text(
                    (maxConsumption - predictionList[0]['current_month_consumption']).toStringAsFixed(2) +
                        " KwH remaining for this month",
                    style: TextStyle(
                        color: Colors.red, fontWeight: FontWeight.bold),
                  ))
                ],
              )),
          ListView(
            physics: NeverScrollableScrollPhysics(),
            padding: const EdgeInsets.all(8),
            shrinkWrap: true,
            children: <Widget>[
              Card(
                  margin: EdgeInsets.all(10),
                  // height: 200,
                  // padding: EdgeInsets.all(20),
                  color: Colors.white,
                  child: Padding(
                    padding: EdgeInsets.all(10),
                    child: Column(
                      children: [
                        Text("Expected power consumption: ${predictionList[0]['current_estimated_consumption']} KwH",
                            style:
                                const TextStyle(fontWeight: FontWeight.bold)),
                        SizedBox(height: 10,),
                        Text("Estimated power bill: ₹ ${predictionList[0]['estimated_bill']}",
                            style:
                            const TextStyle(fontWeight: FontWeight.normal)),
                        SizedBox(
                          height: 20,
                        ),
                        Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(DateFormat.MMMM().format(lastMidnight),
                                style: const TextStyle(color: Colors.black54)),
                            Text("Current Date: "+ lastMidnight.day.toString(),
                                style: const TextStyle(color: Colors.black54))
                          ],
                        ),
                      ],
                    ),
                  )),
              Card(
                  margin: EdgeInsets.all(10),
                  // height: 200,
                  // padding: EdgeInsets.all(20),
                  color: Colors.white,
                  child: Padding(
                    padding: EdgeInsets.all(10),
                    child: Column(
                      children: [
                        Text(
                            "Your limit per month: " +
                                maxConsumption.toString() +
                                " KwH",
                            style:
                                const TextStyle(fontWeight: FontWeight.bold)),
                        SizedBox(
                          height: 20,
                        ),
                        Text("You are currently under your set limit",
                            style: const TextStyle(color: Colors.black54)),
                      ],
                    ),
                  )),
              Card(
                  margin: EdgeInsets.all(10),
                  // height: 200,
                  // padding: EdgeInsets.all(20),
                  color: Colors.white,
                  child: Padding(
                    padding: EdgeInsets.all(10),
                    child: Column(
                      children: [
                        Text(
                            "Solar energy generated this month : ${predictionList[0]['solar_val']} kWh",
                            style:
                            const TextStyle(fontWeight: FontWeight.bold)),

                      ],
                    ),
                  )),
            ],
          ),

          // Stack(
          //   children: <Widget>[
          //     AspectRatio(
          //       aspectRatio: 1.70,
          //       child: Container(
          //         decoration: const BoxDecoration(
          //             borderRadius: BorderRadius.all(
          //               Radius.circular(18),
          //             ),
          //             color: Color(0xffffff)),
          //         child: Padding(
          //           padding: const EdgeInsets.only(
          //               right: 18.0, left: 12.0, top: 24, bottom: 12),
          //           child: LineChart(
          //             mainData(),
          //           ),
          //         ),
          //       ),
          //     ),
          //   ],
          // ),
          Padding(
            padding: const EdgeInsets.only(right: 18.0, left: 18.0, top: 24,bottom: 40),
            child: Text("Solar energy generated today",
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
                      swapAnimationDuration: const Duration(milliseconds: 250),
                    ),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  // @override
  // Widget build(BuildContext context) {
  //   return AspectRatio(
  //     aspectRatio: 1.23,
  //     child: Container(
  //       decoration: const BoxDecoration(
  //         borderRadius: BorderRadius.all(Radius.circular(18)),
  //         gradient: LinearGradient(
  //           colors: [
  //             Color(0xff2c274c),
  //             Color(0xff46426c),
  //           ],
  //           begin: Alignment.bottomCenter,
  //           end: Alignment.topCenter,
  //         ),
  //       ),
  //       child: S,
  //     ),
  //   );
  // }

  LineChartData mainData() {
    return LineChartData(
      gridData: FlGridData(
        show: true,
        drawVerticalLine: true,
        getDrawingHorizontalLine: (value) {
          return FlLine(
            color: const Color(0xff37434d),
            strokeWidth: 1,
          );
        },
        getDrawingVerticalLine: (value) {
          return FlLine(
            color: const Color(0xff37434d),
            strokeWidth: 1,
          );
        },
      ),
      titlesData: FlTitlesData(
        show: true,
        rightTitles: SideTitles(showTitles: false),
        topTitles: SideTitles(showTitles: false),
        bottomTitles: SideTitles(
          showTitles: true,
          reservedSize: 22,
          interval: 1,
          getTextStyles: (context, value) => const TextStyle(
              color: Color(0xff68737d),
              fontWeight: FontWeight.bold,
              fontSize: 16),
          getTitles: (value) {
            switch (value.toInt()) {
              case 1:
                return "Aug 1";
              case 5:
                return 'Aug 15';
              case 10:
                return 'Aug 30';
            }
            return '';
          },
          margin: 8,
        ),
        leftTitles: SideTitles(
          showTitles: true,
          interval: 1,
          getTextStyles: (context, value) => const TextStyle(
            color: Color(0xff67727d),
            fontWeight: FontWeight.bold,
            fontSize: 15,
          ),
          getTitles: (value) {
            switch (value.toInt()) {
              case 1:
                return '1KW';
              case 3:
                return '2KW';
              case 5:
                return '3KW';
            }
            return '';
          },
          reservedSize: 32,
          margin: 12,
        ),
      ),
      borderData: FlBorderData(
          show: true,
          border: Border.all(color: const Color(0xff37434d), width: 1)),
      minX: 0,
      maxX: 11,
      minY: 0,
      maxY: 6,
      lineBarsData: [
        LineChartBarData(
          spots: [
            FlSpot(0, 3),
            FlSpot(2.6, 2),
            FlSpot(4.9, 5),
            FlSpot(6.8, 3.1),
            FlSpot(8, 4),
            FlSpot(9.5, 3),
            FlSpot(11, 4),
          ],
          isCurved: false,
          colors: gradientColors,
          barWidth: 5,
          isStrokeCapRound: true,
          dotData: FlDotData(
            show: false,
          ),
          belowBarData: BarAreaData(
            show: true,
            colors:
                gradientColors.map((color) => color.withOpacity(0.3)).toList(),
          ),
        ),
      ],
    );
  }
}
