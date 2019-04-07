var capture_interval = null;

function take_picture() {
	var now = new Date();
	document.getElementById('previewImage').src = 
		'http://localhost:8081/capture/preview/?' + now.getTime();
}

function capture_next_frame() {
	var now = new Date();
	document.getElementById('captureImage').src = 
		'http://localhost:8081/capture/?' + now.getTime();
}

function localhost_connect() {
	if (capture_interval != null)
		window.clearInterval(capture_interval);
	capture_interval = window.setInterval(capture_next_frame, 750);
}

function localhost_disconnect() {
	window.clearInterval(capture_interval);
	capture_interval = null;
}

