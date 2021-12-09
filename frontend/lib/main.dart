import 'package:capstone/screens/alerts.dart';
import 'package:capstone/screens/forecasts.dart';
import 'package:capstone/screens/livestats.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

void main() {
  // Register all the models and services before the app starts
  WidgetsFlutterBinding.ensureInitialized();
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  final Future<FirebaseApp> _initialization = Firebase.initializeApp();

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
      // Initialize FlutterFire:
      future: _initialization,
      builder: (context, snapshot) {
        // Check for errors
        if (snapshot.hasError) {
          print("Error setting up firebase"+snapshot.error.toString());
        }

        // Once complete, show your application
        if (snapshot.connectionState == ConnectionState.done) {
          return AppLauncher();
        }

        return PlaceHolder();
        // Otherwise, show something whilst waiting for initialization to complete
      },
    );
  }
}



class PlaceHolder extends StatelessWidget{
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        home: Scaffold(
          backgroundColor: Colors.white,
          body: Center(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: <Widget>[
                CircularProgressIndicator(
                  strokeWidth: 3,
                  valueColor: AlwaysStoppedAnimation(
                    Color(0xff19c7c1),
                  ),
                )
              ],
            ),
          ),
        )
    );
  }
}

class AppLauncher extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(primarySwatch: Colors.indigo),
      home: DefaultTabController(
        length: 3,
        child: Scaffold(
          appBar: AppBar(
            bottom: TabBar(
              tabs: [
                Tab(
                  icon: Icon(Icons.device_thermostat),
                  text: 'Forecasts',
                ),
                Tab(
                  icon: Icon(Icons.app_registration),
                  text: 'Live Stats',
                ),
                Tab(
                  icon: Icon(Icons.app_registration),
                  text: 'Alerts',
                ),
              ],
            ),
            title: Text('IoT Center'),
          ),
          body: TabBarView(
            children: [Forecasts(), LiveStats(), Alerts()],
          ),
        ),
      ),
    );
  }
}
