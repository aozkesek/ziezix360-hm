var previewInterval = null;

function start_preview() {

    if (previewInterval != null)
        window.clearInterval(previewInterval);

    previewInterval = window.setInterval(get_next_preview_frame, 500);


}

function take_photo() {
    if (previewInterval != null)
            window.clearInterval(previewInterval);

    var aDate = new Date();
    document.getElementById('bioImage').src =
        'http://localhost:8081/capture/preview/?' + aDate.getTime();
}


function get_next_preview_frame() {
    var aDate = new Date();
    document.getElementById('bioImage').src =
        'http://localhost:8081/capture/?' + aDate.getTime();
}
