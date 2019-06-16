var capture_interval = null;

function take_picture() {
	localhost_disconnect();
	var now = new Date();
	document.getElementById('capturedImage').src = 
		'http://localhost:8081/capture/camera/preview/?' + now.getTime();
}

function capture_next_frame() {
	var now = new Date();
	document.getElementById('capturedImage').src = 
		'http://localhost:8081/capture/camera/?' + now.getTime();
}

function localhost_connect() {
	if (capture_interval != null)
		window.clearInterval(capture_interval);
	capture_interval = window.setInterval(capture_next_frame, 750);
}

function localhost_disconnect() {
	window.clearInterval(capture_interval);
	capture_interval = null;
	document.getElementById('capturedImage').src = '';
}

function take_fingerprint() {
	localhost_disconnect();
	var now = new Date();
	document.getElementById('capturedImage').src = '';
	document.getElementById('capturedImage').src = 
		'http://localhost:8081/capture/fingerprint/?' + now.getTime();	
}