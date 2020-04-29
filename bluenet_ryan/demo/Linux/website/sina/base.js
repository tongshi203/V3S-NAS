/* 1,268,95 2015-06-04 11:54:22 */

var $globalInfo = $globalInfo||{};
if(typeof $globalInfo.SHMLoaded == 'undefined'){
	$globalInfo.SHMLoaded = false;
	var SHM = (function(){
		var it = {};
		//getElementById
		it.E = function(id){
			if (typeof id === "string") {
				return document.getElementById(id);
			}
			return id;
		};
		//createElement
		it.C = function(tag){
			tag = tag.toUpperCase();
			if (tag == 'TEXT') {
				return document.createTextNode('');
			}
			if (tag == 'BUFFER') {
				return document.createDocumentFragment();
			}
			return document.createElement(tag);
		};
		//register
		it.register = function(namespace, method) {
	        var i   = 0,
				un  = it,
				ns  = namespace.split('.'),
				len = ns.length,
				upp = len - 1,
				key;
			while(i<len){
				key = ns[i];
				if(i==upp){
					if(un[key] !== undefined){
						throw ns + ':: has registered';
					}
					un[key] = method(it);
				}
				if(un[key]===undefined){
					un[key] = {}
				}
				un = un[key];
				i++
			}
	    };
		//register short
		it.regShort = function(key, method){
			if (it[key] !== undefined) {
				throw key + ':: has registered';
			}
	        it[key] = method;
		};
		var Detect = function(){
	        var ua = navigator.userAgent.toLowerCase();
	        this.isIE = /msie/.test(ua);
	        this.isOPERA = /opera/.test(ua);
	        this.isMOZ = /gecko/.test(ua);
	        this.isIE5 = /msie 5 /.test(ua);
	        this.isIE55 = /msie 5.5/.test(ua);
	        this.isIE6 = /msie 6/.test(ua);
	        this.isIE7 = /msie 7/.test(ua);
	        this.isCHROME = /chrome/i.test(ua)&&/webkit/i.test(ua)&&/mozilla/i.test(ua);
	        this.isSAFARI = /safari/.test(ua)&&!this.isCHROME;
	        this.iswinXP = /windows nt 5.1/.test(ua);
	        this.iswinVista = /windows nt 6.0/.test(ua);
	        this.isFF = /firefox/.test(ua);
	        this.isIOS = /\((iPhone|iPad|iPod)/i.test(ua);
	    };
	    $globalInfo.ua = new Detect();
		return it;
	})();
}else{
	SHM._register = SHM.register;
	SHM.register = function(m,n){}
}
SHM.register('dom.ready', function(){
	var  fns     = []
		,isReady = 0
		,inited  = 0
		,isReady = 0;

	var checkReady = function(){
		if(document.readyState === 'complete'){
			return 1;
		}
		return isReady;
	};

	var onReady = function(type){
		if(isReady){return}
		isReady = 1;

		if(fns){
			while (fns.length) {
				fns.shift()()
			}
		}
		fns = null
	};

	var bindReady = function(){
		if(inited){return}
		inited = 1;

		//开始初始化domReady函数，判定页面的加载情况
		if (document.readyState === "complete") {
			onReady();
		} else if (document.addEventListener) {
			document.addEventListener("DOMContentLoaded", function() {
				document.removeEventListener("DOMContentLoaded", arguments.callee, false);
				onReady();
			}, false);
			//不加这个有时chrome firefox不起作用
			window.addEventListener( "load", function(){
				window.removeEventListener("load", arguments.callee, false);
				onReady();
			}, false );
		} else {
			document.attachEvent("onreadystatechange", function() {
				if (document.readyState == "complete") {
					document.detachEvent("onreadystatechange", arguments.callee);
					onReady();
				}
			});
			(function() {
				if (isReady) {
					return;
				}
				var node = new Image
				try {
					node.doScroll();
					node = null //防止IE内存泄漏
				} catch (e) {
					setTimeout(arguments.callee, 64);
					return;
				}
				onReady();
			})();
		}
	};

	return function(fn){
		bindReady();
		if(!checkReady()){
			fns.push(fn);
			return;
		}
		//onReady();
		fn.call();
	}
});
SHM.register('util.getUniqueKey', function($){
	return function(){
		return Math.floor(Math.random() * 1000) + new Date().getTime().toString();
	};
});
SHM.register('dom.uniqueID', function($){
	return function(node) {
		return node && (node.uniqueID || (node.uniqueID = $.util.getUniqueKey()));
	};
});
SHM.register('dom.hasClass', function($) {
	return function(a, b) {
		return(new RegExp('(^|\\s)' + b + '($|\\s)')).test(a.className)
	}
});
SHM.register('dom.addClass', function($) {
	return function(b, c) {
		b.nodeType === 1 && ($.dom.hasClass(b, c) || (b.className = $.str.trim(b.className) + ' ' + c))
	}
});
SHM.register('dom.removeClass', function($) {
	return function(b, c) {
		b.nodeType === 1 && $.dom.hasClass(b, c) && (b.className = b.className.replace(new RegExp('(^|\\s)' + c + '($|\\s)'), ' '))
	}
});
SHM.register('dom.getScrollPos', function($){
	return function(doc){
	    doc = doc || document;
	    var dd = doc.documentElement;
	    var db = doc.body;
	    return [
	    		Math.max(dd.scrollTop, db.scrollTop),
	    		Math.max(dd.scrollLeft, db.scrollLeft),
	    		Math.max(dd.scrollWidth, db.scrollWidth),
	    		Math.max(dd.scrollHeight, db.scrollHeight)
	    		];
	}
});
SHM.register('dom.getStyle', function($){
	    var getStyle = function (el, property) {
	    	switch (property) {
	    		// 透明度
	    		case "opacity":
	    			var val = 100;
	    			try {
	    					val = el.filters['DXImageTransform.Microsoft.Alpha'].opacity;
	    			}
	    			catch(e) {
	    				try {
	    					val = el.filters('alpha').opacity;
	    				}catch(e){}
	    			}
	    			return val/100;
	    		 // 浮动
	    		 case "float":
	    			 property = "styleFloat";
	    		 default:
	    			 var value = el.currentStyle ? el.currentStyle[property] : null;
	    			 return ( el.style[property] || value );
	    	}
	    };
	    if(!$globalInfo.ua.isIE) {
			getStyle = function (el, property) {
				// 浮动
				if(property == "float") {
					property = "cssFloat";
				}
				// 获取集合
				try{
					var computed = document.defaultView.getComputedStyle(el, "");
				}
				catch(e) {
					traceError(e);
				}
				return el.style[property] || computed ? computed[property] : null;
			};
		}
	return getStyle;
});

SHM.register('dom.getWinSize', function() {
	return function(win){
		var w, h;
		var target;
		if (win) {
			target = win.document;
		}
		else {
			target = document;
		}
		if (self.innerHeight) { // all except Explorer
			if (win) {
				target = win.self;
			}
			else {
				target = self;
			}
			w = target.innerWidth;
			h = target.innerHeight;
		}
		else if (target.documentElement && target.documentElement.clientHeight) { // Explorer 6 Strict Mode
			w = target.documentElement.clientWidth;
			h = target.documentElement.clientHeight;
		}
		else if (target.body) { // other Explorers
			w = target.body.clientWidth;
			h = target.body.clientHeight;
		}
		return 	{
					width : w,
					height : h
				};
	};
});
SHM.register('dom.getXY', function($){
	var getStyle = $.dom.getStyle;
	var getScrollPos = $.dom.getScrollPos;
	var getXY = function (el) {
		if ((el.parentNode == null || el.offsetParent == null || getStyle(el, "display") == "none") && el != document.body) {
			return false;
		}
		var parentNode = null;
		var pos = [];
		var box;
		var doc = el.ownerDocument;
		// IE
		box = el.getBoundingClientRect();
		var scrollPos = getScrollPos(el.ownerDocument);
		return [box.left + scrollPos[1], box.top + scrollPos[0]];
		// IE end
		parentNode = el.parentNode;
		while (parentNode.tagName && !/^body|html$/i.test(parentNode.tagName)) {
			if (getStyle(parentNode, "display").search(/^inline|table-row.*$/i)) {
				pos[0] -= parentNode.scrollLeft;
				pos[1] -= parentNode.scrollTop;
			}
			parentNode = parentNode.parentNode;
		}
		return pos;
	};
	if(!$globalInfo.ua.isIE) {
		getXY = function (el) {
			if ((el.parentNode == null || el.offsetParent == null || getStyle(el, "display") == "none") && el != document.body) {
				return false;
			}
			var parentNode = null;
			var pos = [];
			var box;
			var doc = el.ownerDocument;

			var isSAFARI = $globalInfo.ua.isSAFARI;

			// FF
			pos = [el.offsetLeft, el.offsetTop];
			parentNode = el.offsetParent;
			var hasAbs = getStyle(el, "position") == "absolute";

			if (parentNode != el) {
				while (parentNode) {
						pos[0] += parentNode.offsetLeft;
						pos[1] += parentNode.offsetTop;
						if (isSAFARI && !hasAbs && getStyle(parentNode,"position") == "absolute" ) {
								hasAbs = true;
						}
						parentNode = parentNode.offsetParent;
				}
			}

			if (isSAFARI && hasAbs) {
				pos[0] -= el.ownerDocument.body.offsetLeft;
				pos[1] -= el.ownerDocument.body.offsetTop;
			}
			parentNode = el.parentNode;
			// FF End
			while (parentNode.tagName && !/^body|html$/i.test(parentNode.tagName)) {
				if (getStyle(parentNode, "display").search(/^inline|table-row.*$/i)) {
					pos[0] -= parentNode.scrollLeft;
					pos[1] -= parentNode.scrollTop;
				}
				parentNode = parentNode.parentNode;
			}
			return pos;
		};
	}
	return getXY;
});
SHM.register('dom.insertAfter', function(a) {
	return function(ele, rEle) {
		var par = rEle.parentNode;
		par.lastChild == rEle ? par.appendChild(ele) : par.insertBefore(ele, rEle.nextSibling)
	}
});
SHM.register('dom.insertBefore', function(a) {
	return function(ele, rEle) {
		var par = rEle.parentNode;
		par.insertBefore(ele, rEle)
	}
});
SHM.register('dom.isNode', function($){
	return function(oNode){
	    return !!((oNode != undefined) && oNode.nodeName && oNode.nodeType);
	}
});
SHM.register('dom.parseDOM', function(a) {
		return function(a) {
			for(var b in a) a[b] && a[b].length == 1 && (a[b] = a[b][0]);
			return a
		}
	});
// SHM.register('dom.sizzle', function(a) {
// 	function c(a, b, c, d, e, f) {for(var g = 0, h = d.length; g < h; g++) {var i = d[g]; if(i) {i = i[a]; var j = !1; while(i) {if(i.sizcache === c) {j = d[i.sizset]; break } if(i.nodeType === 1 && !f) {i.sizcache = c; i.sizset = g } if(i.nodeName.toLowerCase() === b) {j = i; break } i = i[a] } d[g] = j } } } function b(a, b, c, d, e, f) {for(var g = 0, h = d.length; g < h; g++) {var j = d[g]; if(j) {j = j[a]; var k = !1; while(j) {if(j.sizcache === c) {k = d[j.sizset]; break } if(j.nodeType === 1) {if(!f) {j.sizcache = c; j.sizset = g } if(typeof b != "string") {if(j === b) {k = !0; break } } else if(i.filter(b, [j]).length > 0) {k = j; break } } j = j[a] } d[g] = k } } } var d = /((?:\((?:\([^()]+\)|[^()]+)+\)|\[(?:\[[^\[\]]*\]|['"][^'"]*['"]|[^\[\]'"]+)+\]|\\.|[^ >+~,(\[\\]+)+|[>+~])(\s*,\s*)?((?:.|\r|\n)*)/g,
// 		e = 0,
// 		f = Object.prototype.toString,
// 		g = !1,
// 		h = !0;
// 	[0, 0].sort(function() {h = !1; return 0 }); var i = function(a, b, c, e) {c = c || []; b = b || document; var g = b; if(b.nodeType !== 1 && b.nodeType !== 9) return []; if(!a || typeof a != "string") return c; var h = [], l, m, o, p, r = !0, s = i.isXML(b), t = a, u, v, w, x; do {d.exec(""); l = d.exec(t); if(l) {t = l[3]; h.push(l[1]); if(l[2]) {p = l[3]; break } } } while (l); if(h.length > 1 && k.exec(a)) if(h.length === 2 && j.relative[h[0]]) m = q(h[0] + h[1], b); else {m = j.relative[h[0]] ? [b] : i(h.shift(), b); while(h.length) {a = h.shift(); j.relative[a] && (a += h.shift()); m = q(a, m) } } else {if(!e && h.length > 1 && b.nodeType === 9 && !s && j.match.ID.test(h[0]) && !j.match.ID.test(h[h.length - 1])) {u = i.find(h.shift(), b, s); b = u.expr ? i.filter(u.expr, u.set)[0] : u.set[0] } if(b) {u = e ? {expr: h.pop(), set: n(e) } : i.find(h.pop(), h.length === 1 && (h[0] === "~" || h[0] === "+") && b.parentNode ? b.parentNode : b, s); m = u.expr ? i.filter(u.expr, u.set) : u.set; h.length > 0 ? o = n(m) : r = !1; while(h.length) {v = h.pop(); w = v; j.relative[v] ? w = h.pop() : v = ""; w == null && (w = b); j.relative[v](o, w, s) } } else o = h = [] } o || (o = m); o || i.error(v || a); if(f.call(o) === "[object Array]") if(!r) c.push.apply(c, o); else if(b && b.nodeType === 1) for(x = 0; o[x] != null; x++) o[x] && (o[x] === !0 || o[x].nodeType === 1 && i.contains(b, o[x])) && c.push(m[x]); else for(x = 0; o[x] != null; x++) o[x] && o[x].nodeType === 1 && c.push(m[x]); else n(o, c); if(p) {i(p, g, c, e); i.uniqueSort(c) } return c }; i.uniqueSort = function(a) {if(p) {g = h; a.sort(p); if(g) for(var b = 1; b < a.length; b++) a[b] === a[b - 1] && a.splice(b--, 1) } return a }; i.matches = function(a, b) {return i(a, null, null, b) }; i.find = function(a, b, c) {var d; if(!a) return []; for(var e = 0, f = j.order.length; e < f; e++) {var g = j.order[e], h; if(h = j.leftMatch[g].exec(a)) {var i = h[1]; h.splice(1, 1); if(i.substr(i.length - 1) !== "\\") {h[1] = (h[1] || "").replace(/\\/g, ""); d = j.find[g](h, b, c); if(d != null) {a = a.replace(j.match[g], ""); break } } } } d || (d = b.getElementsByTagName("*")); return {set: d, expr: a } }; i.filter = function(a, b, c, d) {var e = a, f = [], g = b, h, k, l = b && b[0] && i.isXML(b[0]); while(a && b.length) {for(var m in j.filter) if((h = j.leftMatch[m].exec(a)) != null && h[2]) {var n = j.filter[m], o, p, q = h[1]; k = !1; h.splice(1, 1); if(q.substr(q.length - 1) === "\\") continue; g === f && (f = []); if(j.preFilter[m]) {h = j.preFilter[m](h, g, c, f, d, l); if(!h) k = o = !0; else if(h === !0) continue } if(h) for(var r = 0; (p = g[r]) != null; r++) if(p) {o = n(p, h, r, g); var s = d ^ !! o; if(c && o != null) s ? k = !0 : g[r] = !1; else if(s) {f.push(p); k = !0 } } if(o !== undefined) {c || (g = f); a = a.replace(j.match[m], ""); if(!k) return []; break } } if(a === e) if(k == null) i.error(a); else break; e = a } return g }; i.error = function(a) {throw "Syntax error, unrecognized expression: " + a }; var j = {order: ["ID", "NAME", "TAG"], match: {ID: /#((?:[\w\u00c0-\uFFFF\-]|\\.)+)/, CLASS: /\.((?:[\w\u00c0-\uFFFF\-]|\\.)+)/, NAME: /\[name=['"]*((?:[\w\u00c0-\uFFFF\-]|\\.)+)['"]*\]/, ATTR: /\[\s*((?:[\w\u00c0-\uFFFF\-]|\\.)+)\s*(?:(\S?=)\s*(['"]*)(.*?)\3|)\s*\]/, TAG: /^((?:[\w\u00c0-\uFFFF\*\-]|\\.)+)/, CHILD: /:(only|nth|last|first)-child(?:\((even|odd|[\dn+\-]*)\))?/, POS: /:(nth|eq|gt|lt|first|last|even|odd)(?:\((\d*)\))?(?=[^\-]|$)/, PSEUDO: /:((?:[\w\u00c0-\uFFFF\-]|\\.)+)(?:\((['"]?)((?:\([^\)]+\)|[^\(\)]*)+)\2\))?/ }, leftMatch: {}, attrMap: {"class": "className", "for": "htmlFor"}, attrHandle: {href: function(a) {return a.getAttribute("href") } }, relative: {"+": function(a, b) {var c = typeof b == "string", d = c && !/\W/.test(b), e = c && !d; d && (b = b.toLowerCase()); for(var f = 0, g = a.length, h; f < g; f++) if(h = a[f]) {while((h = h.previousSibling) && h.nodeType !== 1); a[f] = e || h && h.nodeName.toLowerCase() === b ? h || !1 : h === b } e && i.filter(b, a, !0) }, ">": function(a, b) {var c = typeof b == "string", d, e = 0, f = a.length; if(c && !/\W/.test(b)) {b = b.toLowerCase(); for(; e < f; e++) {d = a[e]; if(d) {var g = d.parentNode; a[e] = g.nodeName.toLowerCase() === b ? g : !1 } } } else {for(; e < f; e++) {d = a[e]; d && (a[e] = c ? d.parentNode : d.parentNode === b) } c && i.filter(b, a, !0) } }, "": function(a, d, f) {var g = e++, h = b, i; if(typeof d == "string" && !/\W/.test(d)) {d = d.toLowerCase(); i = d; h = c } h("parentNode", d, g, a, i, f) }, "~": function(a, d, f) {var g = e++, h = b, i; if(typeof d == "string" && !/\W/.test(d)) {d = d.toLowerCase(); i = d; h = c } h("previousSibling", d, g, a, i, f) } }, find: {ID: function(a, b, c) {if(typeof b.getElementById != "undefined" && !c) {var d = b.getElementById(a[1]); return d ? [d] : [] } }, NAME: function(a, b) {if(typeof b.getElementsByName != "undefined") {var c = [], d = b.getElementsByName(a[1]); for(var e = 0, f = d.length; e < f; e++) d[e].getAttribute("name") === a[1] && c.push(d[e]); return c.length === 0 ? null : c } }, TAG: function(a, b) {return b.getElementsByTagName(a[1]) } }, preFilter: {CLASS: function(a, b, c, d, e, f) {a = " " + a[1].replace(/\\/g, "") + " "; if(f) return a; for(var g = 0, h; (h = b[g]) != null; g++) h && (e ^ (h.className && (" " + h.className + " ").replace(/[\t\n]/g, " ").indexOf(a) >= 0) ? c || d.push(h) : c && (b[g] = !1)); return !1 }, ID: function(a) {return a[1].replace(/\\/g, "") }, TAG: function(a, b) {return a[1].toLowerCase() }, CHILD: function(a) {if(a[1] === "nth") {var b = /(-?)(\d*)n((?:\+|-)?\d*)/.exec(a[2] === "even" && "2n" || a[2] === "odd" && "2n+1" || !/\D/.test(a[2]) && "0n+" + a[2] || a[2]); a[2] = b[1] + (b[2] || 1) - 0; a[3] = b[3] - 0 } a[0] = e++; return a }, ATTR: function(a, b, c, d, e, f) {var g = a[1].replace(/\\/g, ""); !f && j.attrMap[g] && (a[1] = j.attrMap[g]); a[2] === "~=" && (a[4] = " " + a[4] + " "); return a }, PSEUDO: function(a, b, c, e, f) {if(a[1] === "not") if((d.exec(a[3]) || "").length > 1 || /^\w/.test(a[3])) a[3] = i(a[3], null, null, b); else {var g = i.filter(a[3], b, c, !0 ^ f); c || e.push.apply(e, g); return !1 } else if(j.match.POS.test(a[0]) || j.match.CHILD.test(a[0])) return !0; return a }, POS: function(a) {a.unshift(!0); return a } }, filters: {enabled: function(a) {return a.disabled === !1 && a.type !== "hidden"}, disabled: function(a) {return a.disabled === !0 }, checked: function(a) {return a.checked === !0 }, selected: function(a) {a.parentNode.selectedIndex; return a.selected === !0 }, parent: function(a) {return !!a.firstChild }, empty: function(a) {return !a.firstChild }, has: function(a, b, c) {return !!i(c[3], a).length }, header: function(a) {return /h\d/i.test(a.nodeName) }, text: function(a) {return "text" === a.type }, radio: function(a) {return "radio" === a.type }, checkbox: function(a) {return "checkbox" === a.type }, file: function(a) {return "file" === a.type }, password: function(a) {return "password" === a.type }, submit: function(a) {return "submit" === a.type }, image: function(a) {return "image" === a.type }, reset: function(a) {return "reset" === a.type }, button: function(a) {return "button" === a.type || a.nodeName.toLowerCase() === "button"}, input: function(a) {return /input|select|textarea|button/i.test(a.nodeName) } }, setFilters: {first: function(a, b) {return b === 0 }, last: function(a, b, c, d) {return b === d.length - 1 }, even: function(a, b) {return b % 2 === 0 }, odd: function(a, b) {return b % 2 === 1 }, lt: function(a, b, c) {return b < c[3] - 0 }, gt: function(a, b, c) {return b > c[3] - 0 }, nth: function(a, b, c) {return c[3] - 0 === b }, eq: function(a, b, c) {return c[3] - 0 === b } }, filter: {PSEUDO: function(a, b, c, d) {var e = b[1], f = j.filters[e]; if(f) return f(a, c, b, d); if(e === "contains") return(a.textContent || a.innerText || i.getText([a]) || "").indexOf(b[3]) >= 0; if(e === "not") {var g = b[3]; for(var h = 0, k = g.length; h < k; h++) if(g[h] === a) return !1; return !0 } i.error("Syntax error, unrecognized expression: " + e) }, CHILD: function(a, b) {var c = b[1], d = a; switch(c) {case "only": case "first": while(d = d.previousSibling) if(d.nodeType === 1) return !1; if(c === "first") return !0; d = a; case "last": while(d = d.nextSibling) if(d.nodeType === 1) return !1; return !0; case "nth": var e = b[2], f = b[3]; if(e === 1 && f === 0) return !0; var g = b[0], h = a.parentNode; if(h && (h.sizcache !== g || !a.nodeIndex)) {var i = 0; for(d = h.firstChild; d; d = d.nextSibling) d.nodeType === 1 && (d.nodeIndex = ++i); h.sizcache = g } var j = a.nodeIndex - f; return e === 0 ? j === 0 : j % e === 0 && j / e >= 0 } }, ID: function(a, b) {return a.nodeType === 1 && a.getAttribute("id") === b }, TAG: function(a, b) {return b === "*" && a.nodeType === 1 || a.nodeName.toLowerCase() === b }, CLASS: function(a, b) {return(" " + (a.className || a.getAttribute("class")) + " ").indexOf(b) > -1 }, ATTR: function(a, b) {var c = b[1], d = j.attrHandle[c] ? j.attrHandle[c](a) : a[c] != null ? a[c] : a.getAttribute(c), e = d + "", f = b[2], g = b[4]; return d == null ? f === "!=" : f === "=" ? e === g : f === "*=" ? e.indexOf(g) >= 0 : f === "~=" ? (" " + e + " ").indexOf(g) >= 0 : g ? f === "!=" ? e !== g : f === "^=" ? e.indexOf(g) === 0 : f === "$=" ? e.substr(e.length - g.length) === g : f === "|=" ? e === g || e.substr(0, g.length + 1) === g + "-" : !1 : e && d !== !1 }, POS: function(a, b, c, d) {var e = b[2], f = j.setFilters[e]; if(f) return f(a, c, b, d) } } }; i.selectors = j; var k = j.match.POS, l = function(a, b) {return "\\" + (b - 0 + 1) }; for(var m in j.match) {j.match[m] = new RegExp(j.match[m].source + /(?![^\[]*\])(?![^\(]*\))/.source); j.leftMatch[m] = new RegExp(/(^(?:.|\r|\n)*?)/.source + j.match[m].source.replace(/\\(\d+)/g, l)) } var n = function(a, b) {a = Array.prototype.slice.call(a, 0); if(b) {b.push.apply(b, a); return b } return a }; try {Array.prototype.slice.call(document.documentElement.childNodes, 0)[0].nodeType } catch(o) {n = function(a, b) {var c = b || [], d = 0; if(f.call(a) === "[object Array]") Array.prototype.push.apply(c, a); else if(typeof a.length == "number") for(var e = a.length; d < e; d++) c.push(a[d]); else for(; a[d]; d++) c.push(a[d]); return c } } var p; document.documentElement.compareDocumentPosition ? p = function(a, b) {if(!a.compareDocumentPosition || !b.compareDocumentPosition) {a == b && (g = !0); return a.compareDocumentPosition ? -1 : 1 } var c = a.compareDocumentPosition(b) & 4 ? -1 : a === b ? 0 : 1; c === 0 && (g = !0); return c } : "sourceIndex" in document.documentElement ? p = function(a, b) {if(!a.sourceIndex || !b.sourceIndex) {a == b && (g = !0); return a.sourceIndex ? -1 : 1 } var c = a.sourceIndex - b.sourceIndex; c === 0 && (g = !0); return c } : document.createRange && (p = function(a, b) {if(!a.ownerDocument || !b.ownerDocument) {a == b && (g = !0); return a.ownerDocument ? -1 : 1 } var c = a.ownerDocument.createRange(), d = b.ownerDocument.createRange(); c.setStart(a, 0); c.setEnd(a, 0); d.setStart(b, 0); d.setEnd(b, 0); var e = c.compareBoundaryPoints(Range.START_TO_END, d); e === 0 && (g = !0); return e }); i.getText = function(a) {var b = "", c; for(var d = 0; a[d]; d++) {c = a[d]; c.nodeType === 3 || c.nodeType === 4 ? b += c.nodeValue : c.nodeType !== 8 && (b += i.getText(c.childNodes)) } return b }; (function() {var a = document.createElement("div"), b = "script" + (new Date).getTime(); a.innerHTML = "<a name='" + b + "'/>"; var c = document.documentElement; c.insertBefore(a, c.firstChild); if(document.getElementById(b)) {j.find.ID = function(a, b, c) {if(typeof b.getElementById != "undefined" && !c) {var d = b.getElementById(a[1]); return d ? d.id === a[1] || typeof d.getAttributeNode != "undefined" && d.getAttributeNode("id").nodeValue === a[1] ? [d] : undefined : [] } }; j.filter.ID = function(a, b) {var c = typeof a.getAttributeNode != "undefined" && a.getAttributeNode("id"); return a.nodeType === 1 && c && c.nodeValue === b } } c.removeChild(a); c = a = null })(); (function() {var a = document.createElement("div"); a.appendChild(document.createComment("")); a.getElementsByTagName("*").length > 0 && (j.find.TAG = function(a, b) {var c = b.getElementsByTagName(a[1]); if(a[1] === "*") {var d = []; for(var e = 0; c[e]; e++) c[e].nodeType === 1 && d.push(c[e]); c = d } return c }); a.innerHTML = "<a href='#'></a>"; a.firstChild && typeof a.firstChild.getAttribute != "undefined" && a.firstChild.getAttribute("href") !== "#" && (j.attrHandle.href = function(a) {return a.getAttribute("href", 2) }); a = null })(); document.querySelectorAll && function() {var a = i, b = document.createElement("div"); b.innerHTML = "<p class='TEST'></p>"; if(!b.querySelectorAll || b.querySelectorAll(".TEST").length !== 0) {i = function(b, c, d, e) {c = c || document; if(!e && c.nodeType === 9 && !i.isXML(c)) try {return n(c.querySelectorAll(b), d) } catch(f) {} return a(b, c, d, e) }; for(var c in a) i[c] = a[c]; b = null } }(); (function() {var a = document.createElement("div"); a.innerHTML = "<div class='test e'></div><div class='test'></div>"; if( !! a.getElementsByClassName && a.getElementsByClassName("e").length !== 0) {a.lastChild.className = "e"; if(a.getElementsByClassName("e").length === 1) return; j.order.splice(1, 0, "CLASS"); j.find.CLASS = function(a, b, c) {if(typeof b.getElementsByClassName != "undefined" && !c) return b.getElementsByClassName(a[1]) }; a = null } })(); i.contains = document.compareDocumentPosition ? function(a, b) {return !!(a.compareDocumentPosition(b) & 16) } : function(a, b) {return a !== b && (a.contains ? a.contains(b) : !0) }; i.isXML = function(a) {var b = (a ? a.ownerDocument || a : 0).documentElement; return b ? b.nodeName !== "HTML" : !1 }; var q = function(a, b) {var c = [], d = "", e, f = b.nodeType ? [b] : b; while(e = j.match.PSEUDO.exec(a)) {d += e[0]; a = a.replace(j.match.PSEUDO, "") } a = j.relative[a] ? a + "*" : a; for(var g = 0, h = f.length; g < h; g++) i(a, f[g], c); return i.filter(d, c) }; return i });
SHM.register('dom.builder', function($) {
		return function(str, c) {
			var isStr = typeof str == 'string',
				wrap = str;
			if(isStr) {
				wrap = $.C('div');
				wrap.innerHTML = str
			}
			var list, nodes;
			// nodes = $.dom.sizzle("[node-type]", wrap);
			nodes = $.dom.byAttr(wrap,'node-type');
			list = {};
			for(var h = 0, i = nodes.length; h < i; h += 1) {
				var j = nodes[h].getAttribute('node-type');
				list[j] || (list[j] = []);
				list[j].push(nodes[h])
			}
			var box = str;
			if(isStr) {
				box = $.C('buffer');
				while(wrap.childNodes[0]) box.appendChild(wrap.childNodes[0])
			}
			return {
				box: box,
				list: list
			}
		}
	});
SHM.register('str.trim'/*tpa=https://news.sina.com.cn/js/268/index2015/str.trim*/, function($){
	return function(str){
		//return str.replace(/(^\s*)|(\s*$)/g, "");
		//包括全角空格
		return str.replace(/(^[\s\u3000]*)|([\s\u3000]*$)/g, "");
	};
});
SHM.register('str.encodeDoubleByte', function($){
	return function (str) {
		if(typeof str != "string") {
			return str;
		}
		return encodeURIComponent(str);
	};
});
SHM.register('str.encodeHTML', function($) {
	return function(str) {
		if(typeof str != "string") throw "encodeHTML need a string as parameter";
		return str.replace(/\&/g, "&amp;").replace(/"/g, "&quot;").replace(/\</g, "&lt;").replace(/\>/g, "&gt;").replace(/\'/g, "&#39;").replace(/\u00A0/g, "&nbsp;").replace(/(\u0020|\u000B|\u2028|\u2029|\f)/g, "&#32;")
	}
});
SHM.register('str.decodeHTML', function($) {
	return function(str) {
		if(typeof str != "string") throw "decodeHTML need a string as parameter";
		return str.replace(/&quot;/g, '"').replace(/&lt;/g, "<").replace(/&gt;/g, ">").replace(/&#39/g, "'").replace(/&nbsp;/g, "?").replace(/&#32/g, " ").replace(/&amp;/g, "&")
	}
});
SHM.register('str.byteLength', function($){
	return function(str){
		if(typeof str == "undefined"){
			return 0;
		}
		var aMatch = str.match(/[^\x00-\x80]/g);
		return (str.length + (!aMatch ? 0 : aMatch.length));
	};
});
SHM.register('arr.indexOf', function($){
	return function(oElement, aArray){
		if (aArray.indexOf) {
			return aArray.indexOf(oElement);
		}
		var i = 0, len = aArray.length;
		while(i<len){
			if (aArray[i] === oElement) {
				return i;
			}
			i++
		}
		return -1;
	};
});
SHM.register('arr.inArray', function($){
	return function(oElement, aSource){
		return $.arr.indexOf(oElement, aSource) > -1;
	};
});

SHM.register('arr.foreach', function($){
	return function(aArray, insp){
		if (!$.arr.isArray(aArray)) {
			throw 'the foreach function needs an array as first parameter';
		}
		var i = 0, len = aArray.length, ret = [];
		while(i<len){
			var snap = insp(aArray[i], i);
			if(snap === false){break}
			if(snap !== null) {ret[i] = snap}
			i++
		}
		return ret;
	};
});
SHM.register('arr.isArray', function($){
	return function(o){
	  return Object.prototype.toString.call(o) === '[object Array]';
	};
});
SHM.register('json.jsonToQuery',function($){
	var _fdata   = function(data,isEncode){
		data = data == null? '': data;
		data = $.trim(data.toString());
		if(isEncode){
			return encodeURIComponent(data);
		}else{
			return data;
		}
	};
	return function(JSON,isEncode){
		var _Qstring = [];
		if(typeof JSON == "object"){
			for(var k in JSON){
				if(JSON[k] instanceof Array){
					for(var i = 0, len = JSON[k].length; i < len; i++){
						_Qstring.push(k + "=" + _fdata(JSON[k][i],isEncode));
					}
				}else{
					if(typeof JSON[k] != 'function'){
						_Qstring.push(k + "=" +_fdata(JSON[k],isEncode));
					}
				}
			}
		}
		if(_Qstring.length){
			return _Qstring.join("&");
		}else{
			return "";
		}
	};
});
SHM.register('json.queryToJson',function($){
	return function(QS, isDecode){
		var _Qlist = $.str.trim(QS).split("&");
		var _json  = {};
		var _fData = function(data){
			if(isDecode){
				return decodeURIComponent(data);
			}else{
				return data;
			}
		};
		for(var i = 0, len = _Qlist.length; i < len; i++){
			if(_Qlist[i]){
				_hsh = _Qlist[i].split("=");
				_key = _hsh[0];
				_value = _hsh[1];

				// 如果只有key没有value, 那么将全部丢入一个$nullName数组中
				if(_hsh.length < 2){
					_value = _key;
					_key = '$nullName';
				}
				// 如果缓存堆栈中没有这个数据
				if(!_json[_key]) {
					_json[_key] = _fData(_value);
				}
				// 如果堆栈中已经存在这个数据，则转换成数组存储
				else {
					if($.arr.isArray(_json[_key]) != true) {
						_json[_key] = [_json[_key]];
					}
					_json[_key].push(_fData(_value));
				}
			}
		}
		return _json;
	};
});
SHM.register('evt.addEvent',function($){
	return function(elm, evType,func, useCapture) {
		var _el = $.dom.byId(elm);
		if(_el == null){
			throw new Error("addEvent 找不到对象：" + elm);
			return;
		}
		if (typeof useCapture == 'undefined') {
			useCapture = false;
		}
		if (typeof evType == 'undefined') {
			evType = 'click';
		}
		if (_el.addEventListener) {
			_el.addEventListener(evType, func, useCapture);
			return true;
		}
		else if (_el.attachEvent) {
			var r = _el.attachEvent('on' + evType, func);
			return true;
		}
		else {
			_el['on' + evType] = func;
		}
	};
});
SHM.register('evt.removeEvent',function($){
	return function (oElement,sName, fHandler) {
		var _el = $.dom.byId(oElement);
		if(_el == null){
			throw ("removeEvent 找不到对象：" + oElement);
			return;
		}
		if (typeof fHandler != "function") {
			return;
		}
		if (typeof sName == 'undefined') {
			sName = 'click';
		}
		if (_el.addEventListener) {
			_el.removeEventListener(sName, fHandler, false);
		}
		else if (_el.attachEvent) {
			_el.detachEvent("on" + sName, fHandler);
		}
		fHandler[sName] = null;
	};
});
SHM.register('evt.fixEvent',function($){
	return fixEvent = function (e) {
		if(typeof e == 'undefined')e = window.event;
		if (!e.target) {
			e.target = e.srcElement;
			e.pageX = e.x;
			e.pageY = e.y;
		}
		if(typeof e.layerX == 'undefined')e.layerX = e.offsetX;
		if(typeof e.layerY == 'undefined')e.layerY = e.offsetY;
		return e;
	};
});
SHM.register('evt.preventDefault',function($){
	return function (e) {
		var e = e||window.event;
		if ($globalInfo.ua.isIE) {
		    e.returnValue = false;
		} else {
		    e.preventDefault();
		}
	};
});
//byid
SHM.register('dom.byId'/*tpa=https://news.sina.com.cn/js/268/index2015/dom.byId*/,function($){
	return function(id){
        if (typeof id === 'string') {
            return document.getElementById(id);
        }
        else {
            return id;
        }
    };
});
SHM.register('util.browser', function(a) {
	var b = navigator.userAgent.toLowerCase(),
		c = window.external || "",
		d, e, f, g, h, i = function(a) {
			var b = 0;
			return parseFloat(a.replace(/\./g, function() {
				return b++ == 1 ? "" : "."
			}))
		};
	try {
		/windows|win32/i.test(b) ? h = "windows" : /macintosh/i.test(b) ? h = "macintosh" : /rhino/i.test(b) && (h = "rhino");
		if((e = b.match(/applewebkit\/([^\s]*)/)) && e[1]) {
			d = "webkit";
			g = i(e[1])
		} else if((e = b.match(/presto\/([\d.]*)/)) && e[1]) {
			d = "presto";
			g = i(e[1])
		} else if(e = b.match(/msie\s([^;]*)/)) {
			d = "trident";
			g = 1;
			(e = b.match(/trident\/([\d.]*)/)) && e[1] && (g = i(e[1]))
		} else if(/gecko/.test(b)) {
			d = "gecko";
			g = 1;
			(e = b.match(/rv:([\d.]*)/)) && e[1] && (g = i(e[1]))
		}
		/world/.test(b) ? f = "world" : /360se/.test(b) ? f = "360" : /maxthon/.test(b) || typeof c.max_version == "number" ? f = "maxthon" : /tencenttraveler\s([\d.]*)/.test(b) ? f = "tt" : /se\s([\d.]*)/.test(b) && (f = "sogou")
	} catch(j) {}
	var k = {
		OS: h,
		CORE: d,
		Version: g,
		EXTRA: f ? f : !1,
		IE: /msie/.test(b),
		OPERA: /opera/.test(b),
		MOZ: /gecko/.test(b) && !/(compatible|webkit)/.test(b),
		IE5: /msie 5 /.test(b),
		IE55: /msie 5.5/.test(b),
		IE6: /msie 6/.test(b),
		IE7: /msie 7/.test(b),
		IE8: /msie 8/.test(b),
		IE9: /msie 9/.test(b),
		SAFARI: !/chrome\/([\d.]*)/.test(b) && /\/([\d.]*) safari/.test(b),
		CHROME: /chrome\/([\d.]*)/.test(b),
		IPAD: /\(ipad/i.test(b),
		IPHONE: /\(iphone/i.test(b),
		ITOUCH: /\(itouch/i.test(b),
		MOBILE: /mobile/i.test(b)
	};
	return k;
});
SHM.register('dom.position', function($) {
	var b = function(b) {
			var c, d, e, f, g, h;
			c = b.getBoundingClientRect();
			d = $.dom.scrollPos();
			e = b.ownerDocument.body;
			f = b.ownerDocument.documentElement;
			g = f.clientTop || e.clientTop || 0;
			h = f.clientLeft || e.clientLeft || 0;
			return {
				l: parseInt(c.left + d.left - h, 10) || 0,
				t: parseInt(c.top + d.top - g, 10) || 0
			}
		},
		c = function(b, c) {
			var d;
			d = [b.offsetLeft, b.offsetTop];
			parent = b.offsetParent;
			if(parent !== b && parent !== c) while(parent) {
				d[0] += parent.offsetLeft;
				d[1] += parent.offsetTop;
				parent = parent.offsetParent
			}
			if($.util.browser.OPERA != -1 || $.util.browser.SAFARI != -1 && b.style.position == "absolute") {
				d[0] -= document.body.offsetLeft;
				d[1] -= document.body.offsetTop
			}
			b.parentNode ? parent = b.parentNode : parent = null;
			while(parent && !/^body|html$/i.test(parent.tagName) && parent !== c) {
				if(parent.style.display.search(/^inline|table-row.*$/i)) {
					d[0] -= parent.scrollLeft;
					d[1] -= parent.scrollTop
				}
				parent = parent.parentNode
			}
			return {
				l: parseInt(d[0], 10),
				t: parseInt(d[1], 10)
			}
		};
	return function(d, e) {
		if(d == document.body) return !1;
		if(d.parentNode == null) return !1;
		if(d.style.display == "none") return !1;
		var f = $.obj.parseParam({
			parent: null
		}, e);
		if(d.getBoundingClientRect) {
			if(f.parent) {
				var g = b(d),
					h = b(f.parent);
				return {
					l: g.l - h.l,
					t: g.t - h.t
				}
			}
			return b(d)
		}
		return c(d, f.parent || document.body)
	}
});
//byclass
SHM.register('dom.byClass',function($){
	return function(clz,el,tg){
		el = el || document;
		el = typeof el=='string'?$.dom.byId(el):el;
		tg = tg || '*';
		var rs = [];
		clz = " " + clz +" ";
		var cldr = el.getElementsByTagName(tg), len = cldr.length;
		for (var i = 0; i < len; ++ i){
			var o = cldr[i];
			if (o.nodeType == 1){
				var ecl = " " + o.className + " ";
				if (ecl.indexOf(clz) != -1){
					rs[rs.length] = o;
				}
			}
		}
		return rs;
	};
});
//byattr
SHM.register('dom.byAttr',function($){
	return function(node, attname, attvalue){
		var nodes = [];
		attvalue = attvalue||'';
		for(var i = 0, l = node.childNodes.length; i < l; i ++){
			if(node.childNodes[i].nodeType == 1){
				var fit = false;
				if(attvalue){
					fit = (node.childNodes[i].getAttribute(attname) == attvalue);
				}else{
					fit = (node.childNodes[i].getAttribute(attname) !='')
				}
				if(fit){
					nodes.push(node.childNodes[i]);
				}
				if(node.childNodes[i].childNodes.length > 0){
					nodes = nodes.concat(arguments.callee.call(null, node.childNodes[i], attname, attvalue));
				}
			}
		}
		return nodes;
	};
});
SHM.register('dom.contains', function($){
	return function(root, el) {
        if (root.compareDocumentPosition)
             return root === el || !!(root.compareDocumentPosition(el) & 16);
         if (root.contains && el.nodeType === 1){
             return root.contains(el) && root !== el;
         }
         while ((el = el.parentNode)){
             if (el === root){
             	return true;
             }
         }
         return false;
    };
});
// 自定义事件
SHM.register('evt.custEvent', function($) {

	var _custAttr = "__custEventKey__",
		_custKey = 1,
		_custCache = {},
		/**
		 * 从缓存中查找相关对象
		 * 当已经定义时
		 * 	有type时返回缓存中的列表 没有时返回缓存中的对象
		 * 没有定义时返回false
		 * @param {Object|number} obj 对象引用或获取的key
		 * @param {String} type 自定义事件名称
		 */
		_findObj = function(obj, type) {
			var _key = (typeof obj == "number") ? obj : obj[_custAttr];
			return (_key && _custCache[_key]) && {
				obj: (typeof type == "string" ? _custCache[_key][type] : _custCache[_key]),
				key: _key
			};
		};

	return {
		/**
		 * 对象自定义事件的定义 未定义的事件不得绑定
		 * @method define
		 * @static
		 * @param {Object|number} obj 对象引用或获取的下标(key); 必选
		 * @param {String|Array} type 自定义事件名称; 必选
		 * @return {number} key 下标
		 */
		define: function(obj, type) {
			if(obj && type) {
				var _key = (typeof obj == "number") ? obj : obj[_custAttr] || (obj[_custAttr] = _custKey++),
					_cache = _custCache[_key] || (_custCache[_key] = {});
				type = [].concat(type);
				for(var i = 0; i < type.length; i++) {
					_cache[type[i]] || (_cache[type[i]] = []);
				}
				return _key;
			}
		},

		/**
		 * 对象自定义事件的取消定义
		 * 当对象的所有事件定义都被取消时 删除对对象的引用
		 * @method define
		 * @static
		 * @param {Object|number} obj 对象引用或获取的(key); 必选
		 * @param {String} type 自定义事件名称; 可选 不填可取消所有事件的定义
		 */
		undefine: function(obj, type) {
			if (obj) {
				var _key = (typeof obj == "number") ? obj : obj[_custAttr];
				if (_key && _custCache[_key]) {
					if (typeof type == "string") {
						if (type in _custCache[_key]) delete _custCache[_key][type];
					} else {
						delete _custCache[_key];
					}
				}
			}
		},

		/**
		 * 事件添加或绑定
		 * @method add
		 * @static
		 * @param {Object|number} obj 对象引用或获取的(key); 必选
		 * @param {String} type 自定义事件名称; 必选
		 * @param {Function} fn 事件处理方法; 必选
		 * @param {Any} data 扩展数据任意类型; 可选
		 * @return {number} key 下标
		 */
		add: function(obj, type, fn, data) {
			if(obj && typeof type == "string" && fn) {
				var _cache = _findObj(obj, type);
				if(!_cache || !_cache.obj) {
					throw "custEvent (" + type + ") is undefined !";
				}
				_cache.obj.push({fn: fn, data: data});
				return _cache.key;
			}
		},

		/**
		 * 事件删除或解绑
		 * @method remove
		 * @static
		 * @param {Object|number} obj 对象引用或获取的(key); 必选
		 * @param {String} type 自定义事件名称; 可选; 为空时删除对象下的所有事件绑定
		 * @param {Function} fn 事件处理方法; 可选; 为空且type不为空时 删除对象下type事件相关的所有处理方法
		 * @return {number} key 下标
		 */
		remove: function(obj, type, fn) {
			if (obj) {
				var _cache = _findObj(obj, type), _obj;
				if (_cache && (_obj = _cache.obj)) {
					if ($.arr.isArray(_obj)) {
						if (fn) {
							for (var i = 0; i < _obj.length && _obj[i].fn !== fn; i++);
							_obj.splice(i, 1);
						} else {
							_obj.splice(0);
						}
					} else {
						for (var i in _obj) {
							_obj[i] = [];
						}
					}
					return _cache.key;
				}
			}
		},

		/**
		 * 事件触发
		 * @method fire
		 * @static
		 * @param {Object|number} obj 对象引用或获取的(key); 必选
		 * @param {String} type 自定义事件名称; 必选
		 * @param {Any|Array} args 参数数组或单个的其他数据; 可选
		 * @return {number} key 下标
		 */
		fire: function(obj, type, args) {
			if(obj && typeof type == "string") {
				var _cache = _findObj(obj, type), _obj;
				if (_cache && (_obj = _cache.obj)) {
					if(!$.arr.isArray(args)) {
						args = args != undefined ? [args] : [];
					}
					for(var i = 0; i < _obj.length; i++) {
						var fn = _obj[i].fn;
						if(fn && fn.apply) {
							fn.apply($, [{type: type, data: _obj[i].data}].concat(args));
						}
					}
					return _cache.key;
				}
			}
		},
		/**
		 * 销毁
		 * @method destroy
		 * @static
		 */
		destroy: function() {
			_custCache = {};
			_custKey = 1;
		}
	};
});
SHM.register('evt.getEvent', function($) {
	return function() {
		return document.addEventListener ?
		function() {
			var argCallee = arguments.callee,
				firstArg;
			do {
				firstArg = argCallee.arguments[0];
				if(firstArg && (firstArg.constructor == Event || firstArg.constructor == MouseEvent || firstArg.constructor == KeyboardEvent)) return firstArg
			} while (argCallee = argCallee.caller);
			return firstArg
		} : function(argCallee, firstArg, c) {
			return window.event
		}
	}()
});
// 事件委派
SHM.register('evt.delegatedEvent',function($){

	var checkContains = function(list,el){
		for(var i = 0, len = list.length; i < len; i += 1){
			if($.dom.contains(list[i],el)){
				return true;
			}
		}
		return false;
	};

	return function(actEl,expEls,aType){
		if(!$.dom.isNode(actEl)){
			throw 'SHM.evt.delegatedEvent need an Element as first Parameter';
		}
		if(!expEls){
			expEls = [];
		}
		if($.arr.isArray(expEls)){
			expEls = [expEls];
		}
		var evtList = {};
		var aType = aType || 'action-type';
		var bindEvent = function(e){
			var evt = $.evt.fixEvent(e);
			var el = evt.target;
			var type = e.type;
			if(checkContains(expEls,el)){
				return false;
			}else if(!$.dom.contains(actEl, el)){
				return false;
			}else{
				var actionType = null;
				var checkBuble = function(){
					if(evtList[type] && evtList[type][actionType]){
						return evtList[type][actionType]({
							'evt' : evt,
							'el' : el,
							'e' :e,
							'data' : $.json.queryToJson(el.getAttribute('action-data') || '')
						});
					}else{
						return true;
					}
				};
				while(el && el !== actEl){
					if(!el.getAttribute){
						break;
					}
					actionType = el.getAttribute(aType);
					if(checkBuble() === false){
						break;
					}
					el = el.parentNode;
				}

			}
		};
		var that = {};
		/**
		 * 添加代理事件
		 * @method add
		 * @param {String} funcName
		 * @param {String} evtType
		 * @param {Function} process
		 * @return {void}
		 * @example
		 * 		document.body.innerHTML = '<div id="outer"><a href="###" action_type="alert" action_data="test=123">test</a><div id="inner"></div></div>'
		 * 		var a = SHM.core.evt.delegatedEvent($.E('outer'),$.E('inner'));
		 * 		a.add('alert','click',function(spec){window.alert(spec.data.test)});
		 *
		 */
		that.add = function(funcName, evtType, process){
			if(!evtList[evtType]){
				evtList[evtType] = {};
				$.evt.addEvent(actEl,evtType, bindEvent );
			}
			var ns = evtList[evtType];
			ns[funcName] = process;
		};
		/**
		 * 移出代理事件
		 * @method remove
		 * @param {String} funcName
		 * @param {String} evtType
		 * @return {void}
		 * @example
		 * 		document.body.innerHTML = '<div id="outer"><a href="###" action_type="alert" action_data="test=123">test</a><div id="inner"></div></div>'
		 * 		var a = SHM.core.evt.delegatedEvent($.E('outer'),$.E('inner'));
		 * 		a.add('alert','click',function(spec){window.alert(spec.data.test)});
		 * 		a.remove('alert','click');
		 */
		that.remove = function(funcName, evtType){
			if(evtList[evtType]){
				delete evtList[evtType][funcName];
				if($.objIsEmpty(evtList[evtType])){
					delete evtList[evtType];
					$.evt.removeEvent(actEl, bindEvent, evtType);
				}
			}
		};

		that.pushExcept = function(el){
			expEls.push(el);
		};

		that.removeExcept = function(el){
			if(!el){
				expEls = [];
			}else{
				for(var i = 0, len = expEls.length; i < len; i += 1){
					if(expEls[i] === el){
						expEls.splice(i,1);
					}
				}
			}

		};

		that.clearExcept = function(el){
			expEls = [];
		};

		that.destroy = function(){
			for(k in evtList){
				for(l in evtList[k]){
					delete evtList[k][l];
				}
				delete evtList[k];
				$.evt.removeEvent(actEl, bindEvent, k);
			}
		};
		return that;
	};

});
//SHM.register('fun.bind2',function($){
SHM.register('fun.bind2',function($){
	/**
	 * 保留原型扩展
	 * stan | chaoliang@staff.sina.com.cn
	 * @param {Object} object
	 */
	Function.prototype.bind2 = function(object) {
		var __method = this;
		return function() {
		   return __method.apply(object, arguments);
		};
	};
	return function(fFunc, object) {
		var __method = fFunc;
		return function() {
			return __method.apply(object, arguments);
		};
	};

});
SHM.register('io.jsonp', function($) {
	/**
	 * jsonp
	 * @param  {String}   url      url
	 * @param  {String}   params   params
	 * @param  {Function||String} callback 回调函数，当fix为true时，要求为函数名，即字符串
	 * @param  {Boolean}   fix      是否要回调固定函数，默认为为false，在dpc=1时为true
	 */
	return function(url, params, cb, fix) {
		var head = document.getElementsByTagName('head')[0];
		var idStr = url + '&' + params;
		var ojs = $.dom.byId(idStr);
		ojs && head.removeChild(ojs);
		var fun = '';
		var js = $.C('script');
		fix = fix || false;
		if (fix) {
			if (typeof cb == 'string') {
				fun = cb;
			}
		} else {
			//添加时间戳
			url = url + ((url.indexOf('?') == -1) ? '?' : '&') + '_t=' + Math.random();
			//添加回调
			if (typeof cb == 'function') {
				fun = 'fun_' + new Date().getUTCMilliseconds() + ('' + Math.random()).substring(3);
				eval(fun + '=function(res){cb(res)}');
			}
		}
		url = url + '&callback=' + fun;
		//添加参数,放在最后，dpc=1一般放在最后
		url = url + '&' + params;
		js.src = url;
		js.id = idStr;
		js.type = 'text/javascript';
		js.language = 'javascript';
		head.appendChild(js);

	};
});
SHM.register('io.ajax'/*tpa=https://news.sina.com.cn/js/268/index2015/io.ajax*/,function($){
	//TODO
		/**
		 * 创建 XMLHttpRequest 对象
		 */
	return {
		createRequest:function() {
			var request = null;
			try {
				request = new XMLHttpRequest();
			} catch (trymicrosoft) {
				try {
					request = new ActiveXObject("Msxml2.XMLHTTP");
				} catch (othermicrosoft) {
					try {
						request = ActiveXObject("Microsoft.XMLHTTP");
					} catch (failed) {}
				}
			}
			if(request == null){
				throw ("<b>create request failed</b>", {'html':true});
			}
			else {
				return request;
			}
		},
		/**
		 * 请求参数接收
		 *
		 * @param url 必选参数。请求数据的URL，是一个 URL 字符串，不支持数组
		 * @param option 可选参数 {
		 *  onComplete  : Function (Array responsedData),
		 *  onException : Function (),
		 *  returnType : "txt"/"xml"/"json", 返回数据类型
		 *  GET : {}, 通过 GET 提交的数据
		 *  POST : {} 通过 POST 提交的数据
		 * }
		 */
		request : function (url, option) {
			option = option || {};
			option.onComplete = option.onComplete || function () {};
			option.onException = option.onException || function () {};
			option.onTimeout = option.onTimeout || function () {};
			option.timeout = option.timeout? option.timeout: -1;
			option.returnType = option.returnType || "txt";
			option.method = option.method || "get";
			option.data = option.data || {};
			if(typeof option.GET != "undefined" && typeof option.GET.url_random != "undefined" && option.GET.url_random == 0){
				this.rand = false;
				option.GET.url_random = null;
			}
			this.loadData(url, option);
		},
		/**
		 * 载入指定数据
		 * @param {Object} url
		 * @param {Object} option
		 */
		loadData: function (url, option) {
			var request = this.createRequest(), tmpArr = [];
			var _url = new $.util.url(url);

			var timer;
			// 如果有需要 POST 的数据，加以整理
			if(option.POST){
				for (var postkey in option.POST) {
					var postvalue = option.POST[postkey];
					if(postvalue != null){
						tmpArr.push(postkey + '=' + $.str.encodeDoubleByte(postvalue));
					}
				}
			}
			var sParameter = tmpArr.join("&") || "";
			// GET 方式提交的数据都放入地址中
			if (option.GET) {
				for(var key in option.GET){
					if (key != "url_random") {
						_url.setParam(key, $.str.encodeDoubleByte(option.GET[key]));
					}
				}
			}
			if (this.rand != false) {
				// 接口增加随机数
				_url.setParam("rnd", Math.random());
			}

			if (option.timeout > -1) {
				timer = setTimeout(option.onTimeout, option.timeout);
			}

			// 处理回调
			request.onreadystatechange = function() {
				if(request.readyState == 4){
					var response, type = option.returnType;
					try{
						// 根据类型返回不同的响应
						switch (type){
							case "txt":
								response = request.responseText;
								break;
							case "xml":
								if (Core.Base.detect.$IE) {
									response = request.responseXML;
								}
								else {
									var Dparser = new DOMParser();
									response = Dparser.parseFromString(request.responseText, "text/xml");
								}
								break;
							case "json":
									response = eval("(" + request.responseText + ")");
								break;
						}
						option.onComplete(response);
						clearTimeout(timer);
					}
					catch(e){
						option.onException(e.message, _url);
						return false;
					}
				}
			};
			try{
				// 发送请求
				if(option.POST){
					request.open("POST", _url, true);
					request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
					request.send(sParameter);
				}
				else {
					request.open("GET", _url, true);
					request.send(null);
				}
			}
			catch(e){
				option.onException(e.message, _url);
				return false;
			}
		}
	};

});
SHM.register('io.ijax'/*tpa=https://news.sina.com.cn/js/268/index2015/io.ijax*/,function($){
	return {
			/**
			 * 保存缓冲的任务列表
			 */
			arrTaskLists : [],
			/**
			 * 创建 iframe 节点用于载入数据，因为支持双线程，同时建立两个，减少 DOM 操作次数
			 */
			createLoadingIframe: function () {
				if(this.loadFrames != null){
					return false;
				}
				/**
				 * 生成随机 ID 来保证提交到当前页面的数据交互 iframe
				 * L.Ming | liming1@staff.sina.com.cn 2009-01-11
				 */
				var rndId1 = "loadingIframe_thread" + Math.ceil(Math.random() * 10000);
				var rndId2 = "loadingIframe_thread" + Math.ceil((Math.random() + 1) * 10000);
				this.loadFrames = [rndId1, rndId2];

				var iframeSrc = '';
				if($globalInfo.ua.isIE6){
					// ie6 父页面或在iframe页面中设置document.domain后，无论是和当前域名相同还是根域名，一律视为跨域
					iframeSrc = "javascript:void((function(){document.open();document.domain='sina.com.cn';document.close()})())";
				}
			    var html = '<iframe id="' + rndId1 +'" name="' + rndId1 +'" class="invisible"\
			              scrolling="no" src=""\
			              allowTransparency="true" style="display:none;" frameborder="0"\
			              ><\/iframe>\
						  <iframe id="' + rndId2 +'" name="' + rndId2 +'" class="invisible"\
			              scrolling="no" src="'+iframeSrc+'"\
			              allowTransparency="true" style="display:none;" frameborder="0"\
			              ><\/iframe>';
			    //Sina.dom.addHTML(document.body, html); 临时替换
				var oIjaxIframeCnt = $.C("div");
				oIjaxIframeCnt.id = "ijax_iframes";

				oIjaxIframeCnt.innerHTML = html;
				//$Debug("创建 Ijax 需要的 iframe");
				document.body.appendChild(oIjaxIframeCnt);
				// 记录两个 iframe 加载器，默认是空闲状态

				var loadTimer = setInterval($.fun.bind2(function(){
					if($.E(this.loadFrames[0]) != null && $.E(this.loadFrames[1]) != null){
						clearInterval(loadTimer);
						loadTimer = null;
						this.loadingIframe = {
							"thread1" : {
								"container" : $.E(this.loadFrames[0]),
								"isBusy" : false
							},
							"thread2" : {
								"container" : $.E(this.loadFrames[1]),
								"isBusy" : false
							}
						};
						this.loadByList();
					}
				}, this), 10);
			},
			/**
			 * 判断是否可以开始加载数据，必须是两个 iframe 节点可用的情况下
			 */
			isIjaxReady: function () {
				if(typeof this.loadingIframe == "undefined"){
					return false;
				}
				for(var oLoadCnt in this.loadingIframe){
					if(this.loadingIframe[oLoadCnt].isBusy == false){
						this.loadingIframe[oLoadCnt].isBusy = true;
						return this.loadingIframe[oLoadCnt];
					}
				}
				return false;
			},
			/**
			 * 处理请求参数接收
			 *
			 * @param url 必选参数。请求数据的URL，是一个 URL 字符串，不支持数组
			 * @param option 可选参数 {
			 *  onComplete  : Function (Array responsedData),
			 *  onException : Function ();
			 *  GET : {}, 通过 GET 提交的数据
			 *  POST : {} 通过 POST 提交的数据
			 * }
			 */
			request: function (url, option) {
				var oTask = {};
				oTask.url = url;
				oTask.option = option || {};
				this.arrTaskLists.push(oTask);
				if(this.loadFrames == null){
					this.createLoadingIframe();
				}
				else{
					this.loadByList();
				}
			},
			/**
			 * 缓冲列表管理
			 */
			loadByList: function () {
				// 如果等待列表为空，则终止加载
				if (this.arrTaskLists.length == 0) {
					// 重新建立 iframe
					return false;
				}
				// 取得两个加载器的状态，看是否有空闲的
				var loadStatus = this.isIjaxReady();
				if(loadStatus == false){
					return false;
				}
				var newData = this.arrTaskLists[0];
				this.loadData(newData.url, newData.option, loadStatus);
				// 删除列表第一条
				this.arrTaskLists.shift();
			},
			/**
			 * 加载单条数据
			 */
			loadData: function (url, option, loader) {
				var _url = new $.util.url(url);
				if (option.GET) {
					for(var key in option.GET){
						_url.setParam(key, Core.String.encodeDoubleByte(option.GET[key]));
					}
				}
				// 接口设置 Domain
				//_url.setParam("domain", "1");
				// 接口增加随机数
				//modified by stan | chaoliang@staff.sina.com.cn
				//减少不必要的强制更新数据
				//_url.setParam("rnd", Math.random());
				_url = _url.toString();
				// 当前用于加载数据的 iframe 对象
				var ifm = loader.container;
				ifm.listener = $.fun.bind2(function () {
					if(option.onComplete||option.onException){
						try{
							var iframeObject = ifm.contentWindow.document, sResult;
							// 临时函数
							var tArea = iframeObject.getElementsByTagName( 'textarea')[0];
							if (typeof tArea != "undefined") {
								sResult = tArea.value;
							}
							else {
								sResult = iframeObject.body.innerHTML;
							}
							if(option.onComplete){
								option.onComplete(sResult);
							}
							else{
								option.onException();
							}
						}
						catch(e){
							if(option.onException){
								option.onException(e.message, _url.toString());
							}
						}
					}
					loader.isBusy = false;
					$.evt.removeEvent(ifm,"load",ifm.listener);
					this.loadByList();
				},this);

				$.evt.addEvent(ifm,"load", ifm.listener);

				// 如果需要 post 数据
				if(option.POST){
					var oIjaxForm = $.C("form");
					oIjaxForm.id = "IjaxForm";
					oIjaxForm.action = _url;
					oIjaxForm.method = "post";
					oIjaxForm.target = ifm.id;
					for(var oItem in option.POST) {
						var oInput = $.C("input");
						oInput.type = "hidden";
						oInput.name = oItem;
						//oInput.value = $.str.encodeDoubleByte(option.POST[oItem]);
						//encodeDoubleByte就是encodeURIComponent，会把gbk字符转成utf-8造成乱码
						oInput.value = option.POST[oItem];
						oIjaxForm.appendChild(oInput);
					};
					document.body.appendChild(oIjaxForm);
					try{
						oIjaxForm.submit();
					}catch(e){

					}
				}
				else{
					try{
						window.frames(ifm.id).location.href = _url;
					}catch(e){
						ifm.src = _url;
					};
				}
			}
	};
});
SHM.register('io.jsload',function($){
	JsLoad = {};
	(function () {
		function createScripts (oOpts, oCFG) {

			processUrl(oOpts, oCFG);

			var urls = oOpts.urls;
			var i, len = urls.length;
			for(i = 0; i < len; i ++ ) {
				var js = $.C("script");
				js.src = urls[i].url;
				//js.charset = urls[i].charset;
				/*js[$globalInfo.ua.isIE ? "onreadystatechange" : "onload"] = function(){
					if ($globalInfo.ua.isMOZ || this.readyState.toLowerCase() == 'complete' || this.readyState.toLowerCase() == 'loaded') {*/
				js[document.all ? "onreadystatechange" : "onload"] = function() {
					if (/gecko/.test(navigator.userAgent.toLowerCase()) || this.readyState.toLowerCase() == "complete" || this.readyState.toLowerCase() == "loaded") {
						oCFG.script_loaded_num ++;
					}
				};
				document.getElementsByTagName("head")[0].appendChild(js);
			}
		}

		function processUrl(oOpts, oCFG) {
			var urls = oOpts.urls;
			var get_hash = oOpts.GET;

			var i, len = urls.length;
			var key, url_cls,varname,jsvar, rnd;
			for (i = 0; i < len; i++) {
				rnd =  parseInt(Math.random() * 100000000);
				url_cls = new $.util.url(urls[i].url);

				varname = url_cls.getParam("varname") || "requestId_" + rnd;
				if (oOpts.noreturn != true) {
					url_cls.setParam("varname", varname);
				}
				oCFG.script_var_arr.push(varname);

				// jsvar = url_cls.getParam("jsvar") || "requestId_" + rnd;
				// varname = url_cls.getParam('varname') || 'jsvar';
				// if (oOpts.noreturn != true) {
				// 	url_cls.setParam(varname, jsvar);
				// }
				// oCFG.script_var_arr.push(jsvar);

				for(key in get_hash) {
					if(oOpts.noencode == true) {
						url_cls.setParam(key, get_hash[key]);
					}
					else {
						url_cls.setParam(key, $.str.encodeDoubleByte(get_hash[key]));
					}
				}

				urls[i].url = url_cls.toString();
				urls[i].charset = urls[i].charset || oOpts.charset;
			}
		}

		function ancestor (aUrls, oOpts) {

			var _opts = {
				urls: [],
				charset: "utf-8",
				noreturn: false,
				noencode: true,
				timeout: -1,
				POST: {},
				GET: {},
				onComplete: null,
				onException: null
			};

			var _cfg = {
				script_loaded_num: 0,
				is_timeout: false,
				is_loadcomplete: false,
				script_var_arr: []
			};

			_opts.urls = typeof aUrls == "string"? [{url: aUrls}]: aUrls;

			$.util.parseParam(_opts, oOpts);

			createScripts(_opts, _cfg);

			// 定时检查完成情况
			(function () {

				if(_opts.noreturn == true && _opts.onComplete == null)return;
				var i, data = [];
				// 全部完成
				if (_cfg.script_loaded_num == _opts.urls.length) {
					_cfg.is_loadcomplete = true;
					if (_opts.onComplete != null) {
						for(i = 0; i < _cfg.script_var_arr.length; i ++ ) {
							data.push(window[_cfg.script_var_arr[i]]);
						}
						if(_cfg.script_var_arr.length < 2) {
							_opts.onComplete(data[0]);
						}
						else {
							_opts.onComplete(data);
						}
					}
					return;
				}
				// 达到超时
				if(_cfg.is_timeout == true) {
					return;
				}
				setTimeout(arguments.callee, 50);
			})();

			// 超时处理
			if(_opts.timeout > 0) {
				setTimeout(function () {
					if (_cfg.is_loadcomplete != true) {
						if (_opts.onException != null) {
							_opts.onException();
						}
						_cfg.is_timeout = true;
					}
				}, _opts.timeout);
			}
		}

		JsLoad.request = function (aUrls, oOpts) {
			new ancestor(aUrls, oOpts);
		};

	})();
	return JsLoad;
});
SHM.register('util.hideContainer', function($) {
	var tempDiv, create = function() {
			if(!tempDiv) {
				tempDiv = $.C("div");
				tempDiv.style.cssText = "position:absolute;top:-9999px;left:-9999px;";
				document.getElementsByTagName("head")[0].appendChild(tempDiv);
			}
		},
		Obj = {
			appendChild: function(node) {
				if($.dom.isNode(node)) {
					create();
					tempDiv.appendChild(node);
				}
			},
			removeChild: function(node) {
				$.dom.isNode(node) && tempDiv && tempDiv.removeChild(node);
			}
		};
	return Obj;
});
SHM.register('io.cssLoader', function($) {
	var files = {};
	return function(url, id, cb) {
		cb = cb || function() {};
		var isLoaded = function(url, cb) {
				var file = files[url] || (files[url] = {
					loaded: false,
					list: []
				});
				if(file.loaded) {
					cb(url);
					return false
				}
				file.list.push(cb);
				return file.list.length > 1 ? false : true;
			},
			loaded = function(url) {
				var list = files[url].list;
				for(var i = 0; i < list.length; i++){
					list[i](url);
				}
				files[url].loaded = true;
				delete files[url].list;
			};
		if( isLoaded(url, cb)) {
			var link = $.C("link");
			link.setAttribute("rel", "Stylesheet");
			link.setAttribute("type", "text/css");
			link.setAttribute("charset", "utf-8");
			link.setAttribute("href", url);
			document.getElementsByTagName("head")[0].appendChild(link);
			var wrap = $.C("div");
			wrap.id = id;
			$.util.hideContainer.appendChild(wrap);
			var timeout = 3e3,
				checkLoaded = function() {
					if(parseInt($.dom.getStyle(wrap, 'height')) == 42) {
						$.util.hideContainer.removeChild(wrap);
						loaded(url);
					} else if(--timeout > 0) {
						setTimeout(checkLoaded, 10);
					} else {
						$.util.hideContainer.removeChild(wrap);
						delete files[url]
					}
				};
			setTimeout(checkLoaded, 50);
		}
	}
});
/**
 * Cross-domain POST using window.postMessage()
 */
SHM.register('io.html5Ijax', function($) {
    var _add = $.evt.addEvent,
        _remove = $.evt.removeEvent,

        NOOP = function() {},
        RE_URL = /^http\s?\:\/\/[a-z\d\-\.]+/i,
        ID_PREFIX = 'ijax-html5-iframe-',

        /**
         * Message sender class
         */
        MsgSender = function(cfg) {
            cfg = cfg || {};
            this.init(cfg);
        };
        MsgSender.prototype = {
        	ready: false,

        	init: function(cfg) {
        	    if (this.ready) {
        	        return;
        	    }
        	    var self = this,
        	        iframeId, iframeHtml, iframe, loaded, receiver,
        	        proxyUrl = cfg.proxyUrl,
        	        datas = {};
        	    self.onsuccess = cfg.onsuccess || NOOP;
        	    self.onfailure = cfg.onfailure || NOOP;
        	    if (!proxyUrl) {
        	        return;
        	    }

        	    receiver = function(e) {
        	        if (!self.ready || e.origin !== self.target) {
        	        	self.destroy();
        	            return;
        	        }
        	        var ret = e.data;
        	        if (!ret || ret === 'failure') {
        	        	self.destroy();
        	            self.onfailure && self.onfailure();
        	        } else {
        	            self.onsuccess && self.onsuccess(e.data);
        	            self.destroy()
        	        }
        	    };
        	    _add(window, 'message', receiver);

        	    // insert an iframe
        	    iframeId = ID_PREFIX+Date.parse(new Date());
        	    iframeHtml = '<iframe id="' + iframeId + '" name="' + iframeId +
        	        '" src="' + proxyUrl + '" frameborder="0" ' +
        	        'style="width:0;height:0;display:none;"></iframe>';
        	    var oIjaxIframeCnt = $.C("div");
        	    oIjaxIframeCnt.id = ID_PREFIX+"iframes";
        	    oIjaxIframeCnt.innerHTML = iframeHtml;
        	    // document.body.appendChild(oIjaxIframeCnt);
        	    iframe = oIjaxIframeCnt.childNodes[0];
        	    loaded = function() {
        	        self.ready = true;
        	        var src = iframe.src,
        	            matched = src.match(RE_URL);
        	        self.target = (matched && matched[0]) || '*';
        	    };
        	    _add(iframe, 'load', loaded);
        	    document.body.insertBefore(iframe, document.body.firstChild);

        	    self._iframe = iframe;
        	    self._iframeLoaded = loaded;
        	    self._receiver = receiver;
        	},

        	send: function(cfg) {
        	    cfg = cfg || {};
        	    var self = this,
        	        url = cfg.url,
        	        data = cfg.data,
        	        onsuccess = cfg.onsuccess,
        	        onfailure = cfg.onfailure;

        	    if (!url || typeof url !== 'string') {
        	        return;
        	    }
        	    if (onsuccess) {
        	        self.onsuccess = onsuccess;
        	    }
        	    if (onfailure) {
        	        self.onfailure = onfailure;
        	    }

        	    if (!self.ready) {
        	        setTimeout(function() {
        	            self.send(cfg);
        	        }, 50);
        	        return;
        	    }

        	    if (data) {
        	        data += '&_url=' + window.encodeURIComponent(url);
        	    } else {
        	        data = '_url=' + window.encodeURIComponent(url);
        	    }
        	    self._iframe.contentWindow.postMessage(data, self.target);
        	},

        	destroy: function() {
        	    var iframe = this._iframe;
        	    _remove(iframe, 'load', this._iframeLoaded);
        	    iframe.parentNode.removeChild(iframe);
        	    _remove(window, 'message', this._receiver);
        	    this._iframe = null;
        	    this._iframeLoaded = null;
        	    this._receiver = null;
        	}
        };

    return MsgSender;
});
SHM.register('clz.extend',function($){
	return  function(target,source,deep) {
		for (var property in source) {
			target[property] = source[property];
		}
		return target;
	// 	target = target || {};
	// 	var sType = typeof source, i = 1, options;
	// 	if(sType === 'undefined' || sType === 'boolean') {
	// 		deep = sType === 'boolean' ? source : false;
	// 		source = target;
	// 		target = this;
	// 	}
	// 	if( typeof source !== 'object' && Object.prototype.toString.call(source) !== '[object Function]') {
	// 		source = {};
	// 	}
	// 	while(i <= 2) {
	// 		options = i === 1 ? target : source;
	// 		if(options !== null) {
	// 			for(var name in options ) {
	// 				var src = target[name], copy = options[name];
	// 				if(target === copy){
	// 					continue;
	// 				}
	// 				if(deep && copy && typeof copy === 'object' && !copy.nodeType){
	// 					target[name] = this.extend(src || (copy.length !== null ? [] : {}), copy, deep);
	// 				}else if(copy !== undefined){
	// 					target[name] = copy;
	// 				}
	// 			}
	// 		}
	// 		i++;
	// 	}
	// 	return target;
	}
});
SHM.register('util.cookie',function($){
	/**
	 * 读取cookie,注意cookie名字中不得带奇怪的字符，在正则表达式的所有元字符中，目前 .[]$ 是安全的。
	 * @param {Object} cookie的名字
	 * @return {String} cookie的值
	 * @example
	 * var value = co.getCookie(name);
	 */
	var co={};
	co.getCookie = function (name) {
		name = name.replace(/([\.\[\]\$])/g,'\\\$1');
		var rep = new RegExp(name + '=([^;]*)?;','i');
		var co = document.cookie + ';';
		var res = co.match(rep);
		if (res) {
			return unescape(res[1]) || "";
		}
		else {
			return "";
		}
	};

	/**
	 * 设置cookie
	 * @param {String} name cookie名
	 * @param {String} value cookie值
	 * @param {Number} expire Cookie有效期，单位：小时
	 * @param {String} path 路径
	 * @param {String} domain 域
	 * @param {Boolean} secure 安全cookie
	 * @example
	 * co.setCookie('name','sina',null,"")
	 */
	co.setCookie = function (name, value, expire, path, domain, secure) {
			var cstr = [];
			cstr.push(name + '=' + escape(value));
			if(expire){
				var dd = new Date();
				var expires = dd.getTime() + expire * 3600000;
				dd.setTime(expires);
				cstr.push('expires=' + dd.toGMTString());
			}
			if (path) {
				cstr.push('path=' + path);
			}
			if (domain) {
				cstr.push('domain=' + domain);
			}
			if (secure) {
				cstr.push(secure);
			}
			document.cookie = cstr.join(';');
	};

	/**
	 * 删除cookie
	 * @param {String} name cookie名
	 */
	co.deleteCookie = function(name) {
			document.cookie = name + '=;' + 'expires=Fri, 31 Dec 1999 23:59:59 GMT;';
	};
	return co;
});
SHM.register('util.parseParam',function($){
	return function (oSource, oParams) {
		var key;
		try {
			if (typeof oParams != "undefined") {
				for (key in oSource) {
					if (oParams[key] != null) {
						oSource[key] = oParams[key];
					}
				}
			}
		}
		finally {
			key = null;
			return oSource;
		}
	};
});
SHM.register('util.byteLength',function($){
	 return function(str){
		if(typeof str == "undefined"){
			return 0;
		}
		var aMatch = str.match(/[^\x00-\x80]/g);
		return (str.length + (!aMatch ? 0 : aMatch.length));
	};
});
SHM.register('util.url'/*tpa=https://news.sina.com.cn/js/268/index2015/util.url*/,function($){
	Url = function (url){
	    url = url || "";
	    this.url = url;
		this.query = {};
		this.parse();
	};

	Url.prototype = {
		/**
		 * 解析URL，注意解析锚点必须在解析GET参数之前，以免锚点影响GET参数的解析
		 * @param{String} url? 如果传入参数，则将会覆盖初始化时的传入的url 串
		 */
		parse : function (url){
			if (url) {
				this.url = url;
			}
		    this.parseAnchor();
		    this.parseParam();
		},
		/**
		 * 解析锚点 #anchor
		 */
		parseAnchor : function (){
		    var anchor = this.url.match(/\#(.*)/);
		    anchor = anchor ? anchor[1] : null;
		    this._anchor = anchor;
		    if (anchor != null){
		      this.anchor = this.getNameValuePair(anchor);
		      this.url = this.url.replace(/\#.*/,"");
		    }
		},

		/**
		 * 解析GET参数 ?name=value;
		 */
		parseParam : function (){
		    var query = this.url.match(/\?([^\?]*)/);
		    query = query ? query[1] : null;
		    if (query != null){
		      this.url = this.url.replace(/\?([^\?]*)/,"");
		      this.query = this.getNameValuePair(query);
		    }
		 },
		/**
		 * 目前对json格式的value 不支持
		 * @param {String} str 为值对形式,其中value支持 '1,2,3'逗号分割
		 * @return 返回str的分析结果对象
		 */
		getNameValuePair : function (str){
		    var o = {};
		    str.replace(/([^&=]*)(?:\=([^&]*))?/gim, function (w, n, v) {
		     	if(n == ""){return;}
		      	//v = v || "";//alert(v)
		     	//o[n] = ((/[a-z\d]+(,[a-z\d]+)*/.test(v)) || (/^[\u00ff-\ufffe,]+$/.test(v)) || v=="") ? v : (v.j2o() ? v.j2o() : v);
		    	o[n] = v || "";
			});
		    return o;
		 },
		 /**
		  * 从 URL 中获取指定参数的值
		  * @param {Object} sPara
		  */
		 getParam : function (sPara) {
		 	return this.query[sPara] || "";
		 },
		/**
		 * 清除URL实例的GET请求参数
		 */
		clearParam : function (){
		    this.query = {};
		},

		/**
		 * 设置GET请求的参数，当个设置
		 * @param {String} name 参数名
		 * @param {String} value 参数值
		 */
		setParam : function (name, value) {
		    if (name == null || name == "" || typeof(name) != "string") {
				throw new Error("no param name set");
			}
		    this.query = this.query || {};
		    this.query[name]=value;
		},

		/**
		 * 设置多个参数，注意这个设置是覆盖式的，将清空设置之前的所有参数。设置之后，URL.query将指向o，而不是duplicate了o对象
		 * @param {Object} o 参数对象，其属性都将成为URL实例的GET参数
		 */
		setParams : function (o){
		    this.query = o;
		},

		/**
		 * 序列化一个对象为值对的形式
		 * @param {Object} o 待序列化的对象，注意，只支持一级深度，多维的对象请绕过，重新实现
		 * @return {String} 序列化之后的标准的值对形式的String
		 */
		serialize : function (o){
			var ar = [];
			for (var i in o){
			    if (o[i] == null || o[i] == "") {
					ar.push(i + "=");
				}else{
					ar.push(i + "=" + o[i]);
				}
			}
			return ar.join("&");
		},
		/**
		 * 将URL对象转化成为标准的URL地址
		 * @return {String} URL地址
		 */
		toString : function (){
		    var queryStr = this.serialize(this.query);
		    return this.url + (queryStr.length > 0 ? "?" + queryStr : "")
		                    + (this.anchor ? "#" + this.serialize(this.anchor) : "");
		},

		/**
		 * 得到anchor的串
		 * @param {Boolean} forceSharp 强制带#符号
		 * @return {String} 锚anchor的串
		 */
		getHashStr : function (forceSharp){
		    return this.anchor ? "#" + this.serialize(this.anchor) : (forceSharp ? "#" : "");
		}
	};
	return Url;
});
/**
 * 模板
 * @param  {Object} $ SHM
 */
SHM.register('util.template',function($){
	return function(template, data,isDecode){
	    return template.replace(/#\{(.+?)\}/ig, function(){
	        var key = arguments[1].replace(/\s/ig, '');
	        var ret = arguments[0];
	        var list = key.split('||');
	        for (var i = 0, len = list.length; i < len; i += 1) {
	            if (/^default:.*$/.test(list[i])) {
	                ret = isDecode?decodeURIComponent(list[i].replace(/^default:/, '')):list[i].replace(/^default:/, '');
	                break;
	            }
	            else
	                if (data[list[i]] !== undefined) {
	                    ret =isDecode?decodeURIComponent(data[list[i]]):data[list[i]];
	                    break;
	                }
	        }
	        return ret;
	    });
	};
});
/**
 *	log,控制台
 * @param  {Object} $ SHM
 */
SHM.register('app.log'/*tpa=https://news.sina.com.cn/js/268/index2015/app.log*/,function($){
	// var trace = (location.href.indexOf('log=1') !=-1);
	var trace = false;
	return function() {
		if (!trace) return;
		if (typeof console == 'undefined') return;
		var slice = Array.prototype.slice;
		var args = slice.call(arguments, 0);
		args.unshift("* SHM.app.log >>>>>>");
		try{
			console.log.apply(console, args);
		}catch(e){
			console.log(args);
		}

	};
});
/**
 * 截字，包括全角
 * @param  {Object} $ SHM
 */
SHM.register('app.strLeft',function($){
	return function (s, n) {
		var ELLIPSIS = '...';
		var s2 = s.slice(0, n),
			i = s2.replace(/[^\x00-\xff]/g, "**").length,
			j = s.length,
			k = s2.length;
		//if (i <= n) return s2;
		if(i<n){
			return s2;
		}else if(i==n){
			//原样返回
			if(n==j||k==j){
				return s2;
			}else{
				return s.slice(0,n-2)+ELLIPSIS;
			}
		}
		//汉字
		i -= s2.length;
		switch (i) {
			case 0: return s2;
			case n:
				var s4;
				if(n==j){
					s4 = s.slice(0, (n>>1)-1);
					return s4+ELLIPSIS;
				}else{
					s4 = s.slice(0, n>>1);
					return s4;
				}
			default:
				var k = n - i,
					s3 = s.slice(k, n),
					j = s3.replace(/[\x00-\xff]/g, "").length;
				return j ? s.slice(0, k) + arguments.callee(s3, j) : s.slice(0, k);
		}
	};

});
SHM.register('app.strLeft2',function($){
	var byteLen = $.util.byteLength
	return function(str,len){
		var s = str.replace(/\*/g, " ").replace(/[^\x00-\xff]/g, "**");
		str = str.slice(0, s.slice(0, len).replace(/\*\*/g, " ").replace(/\*/g, "").length);
		if(byteLen(str) > len) str = str.slice(0,str.length -1);
		return str;
	};

});
SHM.register('app.splitNum',function($){
	//千分位
	return function(num){
		num = num+"";
		var re=/(-?\d+)(\d{3})/
		while(re.test(num))
		{
		num=num.replace(re,"$1,$2")
		}
		return num;
	}
});
/**
 * 输入框占位
 * @param  {Object} $ SHM
 */
 SHM.register('app.placeholder',function($){
 	$globalInfo.supportPlaceholder = 'placeholder' in document.createElement('input');
 	return function(inputs){
 			function p(input){
 				//如果input不存在或者支持placeholder,返回
 				if(!input||$globalInfo.supportPlaceholder){
 			        return;
 				}
 				//已经初始化，hasPlaceholder为1
 				var hasPlaceholder = input.getAttribute('hasPlaceholder')||0;
 				if(hasPlaceholder=='1'){
 					return;
 				}
 				var toggleTip = function(){
 					defaultValue = input.defaultValue;
 					$.dom.addClass(input,'gray');
 					input.value = text;
 					input.onfocus = function(){
 					    if(input.value === defaultValue || input.value === text){
 					        this.value = '';
 					        $.dom.removeClass(input,'gray');
 					    }
 					}
 					input.onblur = function(){
 					    if(input.value === ''){
 					        this.value = text;
 					        $.dom.addClass(input,'gray');
 					    }
 					}
 				};
 				var simulateTip = function(){
 					var pwdPlaceholder = $.C('input');
 					pwdPlaceholder.type='text';
 					pwdPlaceholder.className = 'pwd_placeholder gray '+input.className;
 					pwdPlaceholder.value=text;
 					pwdPlaceholder.autocomplete = 'off';
 					input.style.display='none';
 		            input.parentNode.appendChild(pwdPlaceholder);
 		            pwdPlaceholder.onfocus = function(){
 		                pwdPlaceholder.style.display = 'none';
 		                input.style.display = '';
 		                input.focus();
 		            }
 		            input.onblur = function(){
 		                if(input.value === ''){
 		                    pwdPlaceholder.style.display='';
 		                    input.style.display='none';
 		                }
 		            }
 				}
 				//如果没有placeholder或者没有placeholder值，返回
 				var text = input.getAttribute('placeholder');
 				if(!text){
 					//ie10 下的ie7 无法用input.getAttribute('placeholder')取到placeholder值，奇怪！
 					if(input.attributes&&input.attributes.placeholder){
 						text=input.attributes.placeholder.value;
 					}
 				}
 				var tagName = input.tagName;
 				if(tagName=='INPUT'){
 					var inputType = input.type;
 					if(inputType == 'password'&&text){
 						simulateTip();
 					}else if(inputType=='text'&&text){
 						toggleTip();
 					}
 				}else if(tagName=='TEXTAREA'){
 					toggleTip();
 				}
 				input.setAttribute('hasPlaceholder','1');
 			}
 			for (var i = inputs.length - 1; i >= 0; i--) {
 				var input = inputs[i]
 				p(input);
 			};
 		};

 });

SHM.register('app.uaTrack', function($) {
	var prefix = 'index_new_';
	window.SHMUATrack = function(key,val,hasPrefix){
		if(typeof _S_uaTrack == 'function'){
			hasPrefix = hasPrefix||true;
			var key = hasPrefix?prefix+key:key;
			try{
				_S_uaTrack(key, val);
			}catch(e){}
		}
	};
	return SHMUATrack;
});
SHM.register("app.simSelect", function($) {
	var byId = $.dom.byId,
		addEvent = $.evt.addEvent,
		removeEvent = $.evt.removeEvent,
		uatrack = $.app.uaTrack;

	var	sim_select = function(o,more) {
			o = byId(o);
			o.style.display = 'none';
			var opts = o.options,
				parent = o.parentNode,
				self = this;
			self.more = more;
			self.isShow = false;
			self.div = $.C('div');
			self.lineDiv = $.C('div');
			self.tmpDiv = $.C('div');
			self.ul = $.C('ul');
			self.h3 = $.C('h3');
			self.div.className = 'sim-select clearfix';
			parent.replaceChild(self.div, o);
			self.div.appendChild(o);
			self.h3.innerHTML = opts[o.selectedIndex].innerHTML;
			for (var i = 0,
			l = o.length; i < l; i++) {
				var li = $.C('li');
				li.innerHTML = opts[i].innerHTML;
				self.ul.appendChild(li);
				li.onmouseover = function() {
					this.className += ' over'
				};
				li.onmouseout = function() {
					this.className = this.className.replace(/over/gi, '')
				};
				li.onclick = (function(i) {
					return function() {
						self.hide();
						self.h3.innerHTML = this.innerHTML;
						o.selectedIndex = i;
						if (o.onchange) {
							o.onchange();
						}
						if(!self.more) {
							uatrack('search','search_list_click');
						}
					}
				})(i);
			};
			//添加更多选项
			if(!self.more){
				var li = $.C('li');
				li.innerHTML = '<a href="-c=more"/*tpa=http://search.sina.com.cn/?c=more*/ target="_blank">\u66f4\u591a&gt;&gt;</a>';
				li.onmouseover = function() {
					this.className += ' over';
				};
				li.onmouseout = function() {
					this.className = this.className.replace(/over/gi, '');
				};
				self.ul.appendChild(li);
			}
			self.tmpDiv.className = 'sim-ul-flt';
			self.tmpDiv.style.display = 'none';
			self.tmpDiv.innerHTML = '<div class="sim-ul-bg"></div>';
			self.tmpDiv.appendChild(self.ul);
			self.lineDiv.className = 'v-line';
			self.div.appendChild(self.h3);
			self.div.appendChild(self.lineDiv);
			self.div.appendChild(self.tmpDiv);
			self.tmpDiv.style.top = self.h3.offsetHeight + 'px';
			self.tmpDiv.style.width = (self.h3.offsetWidth > 2 ? (self.h3.offsetWidth - 2) : self.h3.offsetWidth) + 'px';
			self.init()
		};

	sim_select.prototype = {
		init: function() {
			var self = this;
			addEvent(document.documentElement, 'click',function(e) {
				self.close(e)
			});
			this.h3.onclick = function() {
				self.toggles()
			}
		},
		show: function() {
			this.tmpDiv.style.display = 'block';
			this.tmpDiv.style.visibility = 'visible';
			this.isShow = true;
			if(!this.more) {
				uatrack('search','search_list_show');
			}

		},
		hide: function() {
			this.tmpDiv.style.display = 'none';
			this.tmpDiv.style.visibility = 'hidden';
			this.isShow = false
		},
		close: function(e) {
			var t = window.event ? window.event.srcElement : e.target;
			do {
				if (t == this.div) {
					return
				} else if (t == document.documentElement) {
					this.hide();
					return
				} else {
					t = t.parentNode;
					if (!t) {
					    break;
					}
				}
			} while (t.parentNode)
		},
		toggles: function() {
			this.isShow ? this.hide() : this.show()
		}
	};

	return sim_select;

});
SHM.register('app.getTextareaData',function($){
	var byAttr = $.dom.byAttr;
	var rendered = '__hasTARendered__';
	var name = 'data-textarea';
	return function(ele,render){
		render = render||false;
		if(!ele){
			return '';
		}
		if(typeof ele[rendered]=='undefined')
		var textarea = byAttr(ele,'node-type',name)[0];
		if(!textarea){
			return '';
		}
		var val = textarea.value;
		if(render){
			var temp = $.C('div');
			var par = textarea.parentNode;
			temp.className = name+'-wrap';
			temp.innerHTML = val;
			var imgs = temp.getElementsByTagName('img');
			if(imgs&&imgs.length>0){
				for (var i = imgs.length - 1; i >= 0; i--) {
					var img = imgs[i];
					var src = img.getAttribute('data-src');
					if(src){
						img.src=src;
						img.removeAttribute('data-src');
					}
				};
			}
			par.insertBefore(temp, textarea);
			par.removeChild(textarea);
		}
		ele[rendered] = true;
		return val;
	};
});
SHM.register('app.tab'/*tpa=https://news.sina.com.cn/js/268/index2015/app.tab*/, function($) {
	var inArray = $.arr.inArray;
	var dom = $.dom;
	var docbody = null;
	var byAttr = dom.byAttr;
	var queryToJson = $.json.queryToJson;
	var hasClass = dom.hasClass;
	var addClass = dom.addClass;
	var removeClass = dom.removeClass;
	var attrName = 'tab-type';
	var eventType = 'mouseover';
	var dlgevt = null;
	var o = {};
	var hasTouch = (typeof(window.ontouchstart) !== 'undefined');
	if(hasTouch){
		eventType = 'touchstart';
	}
	var byTabAttr = function(wrap,arrValue){
		var wraps = byAttr(wrap,attrName,'tab-wrap');
		var eles = byAttr(wrap, attrName, arrValue);
		var elesFilted = [];
		var elesInWraps = [];
		if(wraps.length!=0){
			//提取容器中其它选项卡的项
			for (var i = wraps.length - 1; i >= 0; i--) {
				var wrap = wraps[i];
				var items = byAttr(wrap, attrName, arrValue);
				elesInWraps = elesInWraps.concat(items);
			};
			//过滤掉其它选项卡的项
			for (var i = eles.length - 1; i >= 0; i--) {
				var item = eles[i];
				if(inArray(item,elesInWraps)){
					eles.splice(i,1);
					// delete eles[i];
				}
			};
		}
		return eles;

	};
	var getParent = function(ele,attr,val){
		var par = ele.parentNode;
		if(!par){
			return docbody;
		}
		if(par == docbody||par.getAttribute(attr)==val){
			return par;
		}else{
			return arguments.callee(par,attr,val);
		}
	};
	var getTextareaData = function(ele){
		if(ele){
			var textarea = ele.getElementsByTagName();
		}
	};
	var touchInfo={};
	var bindEventIOS = function(){
		if(!hasTouch){
			return;
		}
		if(typeof(window.ontouchstart) === 'undefined'){
			return;
		}
		var setOpacity = function(ele, opacity) {
			if (typeof(ele.style.opacity) != 'undefined') {
				ele.style.opacity = opacity;
			} else {
				ele.style.filter = 'Alpha(Opacity=' + (opacity * 100) + ')';
			}
		};
		var show = function(ele, time) {
			setOpacity(ele, 0);
			if (!time) {
				time = 40;
			};
			var opacity = 0,
				step = time / 20;
			clearTimeout(ele._showTimeOutId_);
			ele._showTimeOutId_ = setTimeout(function() {
				if (opacity >= 1) {
					return;
				};
				opacity += 1 / step;
				setOpacity(ele, opacity);
				ele._showTimeOutId_ = setTimeout(arguments.callee, 20);
			}, 20)
		};
		var getCurI = function(i,len,direction){
			if(direction=='prev'){
				i = i-1;
				if(i<0){
					i=len-1;
				}
			}else{
				i = i+1;
				if(i==len){
					i=0;
				}

			}
			return i;
		};
		var move = function(ele,direction){
			var clz = 'selected';
			var name = 'tab';

			var wrap = getParent(ele,attrName,'tab-wrap');
			if(!wrap){
				wrap = docbody;
			}
			var data = queryToJson(wrap.getAttribute('tab-data') || '');
			if(data){
				clz = data.clz||clz;
				name = data.name||name;
			}
			// var navs = byAttr(wrap, attrName, name+'-nav');
			// var conts = byAttr(wrap, attrName, name+'-cont');
			var navs = byTabAttr(wrap, name+'-nav');
			var conts = byTabAttr(wrap, name+'-cont');
			if(navs.length!=conts.length){
				return;
			}
			for (var i = 0,len = conts.length;i<len; i++) {
				var nav = navs[i];
				var cont = conts[i];
				if(cont==ele){
					var _index = getCurI(i,len,direction);
					var _nav = navs[_index];
					//如果这个标签不显示，那么就不切换
					if(_nav&&_nav.style.display=='none'){
						break;
					}
					removeClass(nav,clz);
					if(cont){
						cont.style.display = 'none';
					}
					addClass(_nav,clz);
					var _cont = conts[_index];
					if(_cont){
						$.app.getTextareaData(_cont,true);
						_cont.style.display = '';
						show(_cont,200);
					}
				}
			};
		};
		dlgevt.add('tab-cont', 'touchstart', function(e){
			touchstart(e);
		});
		dlgevt.add('tab-cont', 'touchmove', function(e){
			touchmove(e);
			// $.evt.preventDefault(e.evt);
		});
		dlgevt.add('tab-cont', 'touchend', function(e){
			touchend(e);
		});

		touchInfo.iPadStatus = 'ok';
		var touchstart = function(e){
			touchInfo.iPadX = e.evt.touches[0].pageX;
			touchInfo.iPadScrollX = window.pageXOffset;
			touchInfo.iPadScrollY = window.pageYOffset; //用于判断页面是否滚动
		};
		var touchend = function(e){
			if(touchInfo.iPadStatus != 'touch'){return};
			touchInfo.iPadStatus = 'ok';
			//self._state = 'ready';
			var cX = touchInfo.iPadX - touchInfo.iPadLastX;
			if(cX<0){
				move(e.el,'prev');
			}else{
				move(e.el,'next');
			};
		};
		var touchmove = function(e){
			if(e.evt.touches.length > 1){ //多点触摸
				touchend(e);
			};
			touchInfo.iPadLastX = e.evt.touches[0].pageX;
			var cX = touchInfo.iPadX - touchInfo.iPadLastX;
			if(touchInfo.iPadStatus == 'ok'){
				if(touchInfo.iPadScrollY == window.pageYOffset && touchInfo.iPadScrollX == window.pageXOffset && Math.abs(cX)>20){ //横向触摸
					touchInfo.iPadStatus = 'touch';
				}else{
					return;
				};
			};
		};

	};
	o.switchByEle = function(ele){
		if(!ele){
			return;
		}
		var index = 0;
		var clz = 'selected';
		var name = 'tab';

		var wrap = getParent(ele,attrName,'tab-wrap');
		if(!wrap){
			wrap = docbody;
		}
		var data = queryToJson(wrap.getAttribute('tab-data') || '');
		if(data){
			clz = data.clz||clz;
			name = data.name||name;
		}
		// var navs = byAttr(wrap, attrName, name+'-nav');
		// var conts = byAttr(wrap, attrName, name+'-cont');
		var navs = byTabAttr(wrap, name+'-nav');
		var conts = byTabAttr(wrap, name+'-cont');
		if(navs.length!=conts.length){
			return;
		}
		for (var i = 0,len = navs.length;i<len; i++) {
			var nav = navs[i];
			var cont = conts[i];
			if(hasClass(nav,clz)){
				removeClass(nav,clz);
				if(cont){
					cont.style.display = 'none';
				}
			}
			if(nav==ele||cont==ele){
				index = i;
				addClass(nav,clz);
				if(cont){
					$.app.getTextareaData(cont,true);
					cont.style.display = '';
				}
			}
		};
	};
	o.init = function(){
		docbody = document.body;
		dlgevt = $.evt.delegatedEvent(docbody,null,attrName);
		bindEventIOS();
		dlgevt.add('tab-nav', eventType, function(e){
			$.evt.preventDefault(e.evt);
			var ele = e.el;
			o.switchByEle(ele);

		});

	};
	return o;
});
