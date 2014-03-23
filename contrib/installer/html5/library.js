//"use strict";

var OwnLib = {
	$OwnLib__deps: [],
	$OwnLib: {
	},
	_jsOpenURL: function (url) {
		var win = window.open(Pointer_stringify(url), '_blank');
		win.focus();
	},
	_jsAlert: function (reason) {
		alert(Pointer_stringify(reason));
	},
	_jsBacktrace: function () {
		alert(new Error().stack);
	}
};

autoAddDeps(OwnLib, '$OwnLib');
mergeInto(LibraryManager.library, OwnLib);
