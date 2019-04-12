var previewInterval = null;

function start_preview() {

    if (previewInterval != null)
        window.clearInterval(previewInterval);

    previewInterval = window.setInterval(get_next_preview_frame, 3000);


}

function take_photo() {
    if (previewInterval != null)
            window.clearInterval(previewInterval);

    var aDate = new Date();
    document.getElementById('bioImage').src =
        'http://localhost:8081/capture/camera/preview/?' + aDate.getTime();
}


function get_next_preview_frame() {
    var aDate = new Date();
    document.getElementById('bioImage').src =
        'http://localhost:8081/capture/camera/?' + aDate.getTime();
}

function take_fingerprint() {
    if (previewInterval != null)
            window.clearInterval(previewInterval);

    var aDate = new Date();
    document.getElementById('bioImage').src =
        'http://localhost:8081/capture/fingerprint/?' + aDate.getTime();
}
