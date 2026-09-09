// Emscripten JS library (linked via em_link_js_library).
// Image decoding is configured on Module in shell.html.in (noImageDecoding),
// not here — js-library files do not reliably mutate the runtime Module.

mergeInto(LibraryManager.library, {
	_jsOpenURL: function (url, newwindow) {
		var target = newwindow ? '_blank' : '_self';
		var win = window.open(UTF8ToString(url), target);
		if (win)
			win.focus();
	},
	_jsAlert: function (reason) {
		alert(UTF8ToString(reason));
	},
	_jsBacktrace: function () {
		console.error(new Error().stack);
	}
});
