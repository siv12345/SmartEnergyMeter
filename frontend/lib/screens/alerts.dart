import 'dart:convert';

import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';

class Alerts extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _AlertsState();
}

class _AlertsState extends State<Alerts> {
  var alertList = [];

  Future<void> getData() async {
    // Get docs from collection reference
    FirebaseFirestore firestore = FirebaseFirestore.instance;

    CollectionReference alerts = firestore.collection('alerts');

    QuerySnapshot querySnapshot = await alerts.get();

    // Get data from docs and convert map to List
    setState(() {
      alertList = querySnapshot.docs
          .map((doc) => jsonDecode(doc.data()['influx'].toString()))
          .toList();
    });

    print(alertList);
  }

  void initState() {
    super.initState();
    getData();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        body: ListView.builder(
      itemCount: alertList.length,
      itemBuilder: (context, index) {
        return Card(
            margin: EdgeInsets.all(10),
            // height: 200,
            // padding: EdgeInsets.all(20),
            color: Colors.white70,
            child: Padding(
              padding: EdgeInsets.all(10),
              child: Column(
                children: [
                  Text(alertList[index]['_check_name'],
                      style: const TextStyle(fontWeight: FontWeight.bold)),
                  Text("Severity: " + alertList[index]['_level'],
                      style: const TextStyle(fontWeight: FontWeight.normal)),
                  SizedBox(
                    height: 10,
                  ),
                  Text(
                    "Event occured at:  " +
                        DateFormat('yyyy-MM-dd hh:mm')
                            .format(DateTime.parse(alertList[index]['_start'])),
                    style: const TextStyle(color: Colors.black54),
                    textAlign: TextAlign.center,
                  ),
                  SizedBox(
                    height: 10,
                  ),
                  Text(
                    alertList[index]['power'].toString() == 'null'
                        ? "Last voltage reading before drop:   " +
                            alertList[index]['voltage'].toStringAsFixed(2) +
                            " V"
                        : "Last power reading before drop:   " +
                            alertList[index]['power'].toStringAsFixed(2) +
                            " W",
                    style: const TextStyle(
                        fontWeight: FontWeight.w500, color: Colors.blue),
                    textAlign: TextAlign.center,
                  ),
                  Divider(
                    height: 20,
                    thickness: 1,
                    indent: 20,
                    endIndent: 20,
                  ),
                ],
              ),
            ));
      },
    ));
  }
}
