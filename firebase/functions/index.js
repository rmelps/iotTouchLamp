const functions = require('firebase-functions');

const admin = require('firebase-admin');
admin.initializeApp(functions.config().firebase);

var sn = "";
const genericError = "error\r\n"

exports.getColor = functions.https.onRequest((request, response) => {
	// Get SN from lamp making the request
	// send error if serial number not in the query string
	
	if (request.query.s != null) {
		sn = request.query.s;
	} else {
		response.send(genericError);
	}

	admin.database().ref("SN/" + sn).once("value").then(function(snapshot){
		const group = snapshot.val();
		
		admin.database().ref("groups/" + group).once("value").then(function(snap){
			const color = snap.val();
			const red = color["r"];
			const green = color["g"];
			const blue = color["b"];

			response.send("r " + red + "\r\n" + "g " + green + "\r\n" + "b " + blue + "\r\n");
		});
		
	});
});

exports.setColor = functions.https.onRequest((request, response) => {
	var red = " ";
	var green = " ";
	var blue = " ";

	if (request.method != "POST") {
		response.send(genericError);
	}

	if (request.query.s != null) {
		sn = request.query.s;
	} else {
		response.send(genericError);
	}

	if (request.query.r != null) {
		red = request.query.r;
	} else {
		response.send(genericError);
	}

	if (request.query.g != null) {
		green = request.query.g;
	} else {
		response.send(genericError);
	}

	if (request.query.b != null) {
		blue = request.query.b;
	} else {
		response.send(genericError);
	}

	admin.database().ref("SN/" + sn).once("value").then(function(snapshot){
		const group = snapshot.val();
		const ref = admin.database().ref("groups/" + group);          

		ref.once("value").then(function(snap){
			const original = snap.val();
			colors = {"r": red, "g": green, "b": blue};

			ref.set(colors);
			response.send("OK\r\n");
		});
	});
});
