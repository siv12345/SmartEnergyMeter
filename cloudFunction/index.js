var axios = require("axios");
var fs = require("fs");

exports.helloWorld = (req, res) => {
  let message = req.query.message || req.body.message || "Pushing Data to Firebase";
  res.status(200).send(message);

  const myJSON = JSON.stringify(req.body);


  var data = JSON.stringify({
    fields: {
      influx: {
        // stringValue: myJSON.replace(/['"]+/g, '')
        stringValue: myJSON
      },
    },
  });

  var config = {
    method: "post",
    url: "https://firestore.googleapis.com/v1/projects/smartenergymeter/databases/(default)/documents/alerts/",
    headers: {
      key: "API_KEY",
      "Content-Type": "application/json",
    },
    data: data,
  };

  axios(config)
    .then(function (response) {
      console.log(JSON.stringify(response.data));
    })
    .catch(function (error) {
      console.log(error);
    });
};
