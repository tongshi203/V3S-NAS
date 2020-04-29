

(function (W, undefined) {
	var navigator = W.navigator,
	document = W.document,
	documentElement = document.documentElement,
	_cQuery = W.cQuery,
	_push = Array.prototype.push,
	_slice = Array.prototype.slice,
	_indexOf = Array.prototype.indexOf,
	_toString = Object.prototype.toString,
	_hasOwn = Object.prototype.hasOwnProperty,
	_trim = String.prototype.trim,
	_extend = function (dest, src, overwrite) {
		if (!src || !dest || dest === src) {
			return dest || null
		}
		var key;
		if (overwrite) {
			for (key in src) {
				dest[key] = src[key]
			}
		} else {
			for (key in src) {
				if (!dest.hasOwnProperty(key)) {
					dest[key] = src[key]
				}
			}
		}
		return dest
	},
	_createObject = Object.create ? function (proto, c) {
		return Object.create(proto, {
			constructor : {
				value : c
			}
		})
	}
	 : function (proto, c) {
		function F() {}

		F.prototype = proto;
		var o = new F();
		o.constructor = c;
		return o
	},
	_inherit = function (derived, base, px, sx) {
		if (!base || !derived) {
			return derived
		}
		var baseProto = base.prototype,
		derivedProto,
		key,
		item;
		derivedProto = _createObject(baseProto, derived);
		derived.prototype = _extend(derivedProto, derived.prototype, true);
		derived.superclass = _createObject(baseProto, base);
		if (px) {
			_extend(derivedProto, px, true)
		}
		if (sx) {
			_extend(derived, sx, true)
		}
		return derived
	},
	cQuery = function () {};
	cQuery.prototype = {
		constructor : cQuery,
		author : "chenqiang",
		version : "0.0.0"
	};
	_extend(cQuery, {
		noop : function () {},
		guid : 0,
		extend : function (namespace, source, overwrite) {
			if (namespace === null || namespace === "") {
				if (this.isPlainObject(source)) {
					_extend(this, source, overwrite)
				} else {
					throw "lost of some namespace!"
				}
			} else {
				if (this.isString(namespace)) {
					var ns = namespace.split("."),
					len = ns.length,
					i = 0,
					l = len - 1,
					o = this;
					while (i < len) {
						var key = ns[i];
						if (o[key] === undefined) {
							o[key] = {}

						}
						if (i === l) {
							try {
								if (this.isPlainObject(source)) {
									_extend(o[key], source, overwrite)
								} else {
									if (this.isFunction(source)) {
										o[key] = source.apply(cQuery, [cQuery].concat(arguments))
									}
								}
							} catch (e) {}

						}
						o = o[key];
						i++
					}
				}
			}
		},
		isString : function (obj) {
			return typeof(obj) === "string"
		},
		isFunction : function (obj) {
			return typeof(obj) === "function"
		},
		isArray : Array.isArray || function (obj) {
			return Object.prototype.toString.call(obj) === "[object Array]"
		},
		isNumeric : function (obj) {
			return !isNaN(parseFloat(obj)) && isFinite(obj)
		},
		isPlainObject : function (obj) {
			if (!obj || typeof(obj) !== "object" || obj.nodeType || obj === W) {
				return false
			}
			try {
				if (obj.constructor && !_hasOwn.call(obj, "constructor") && !_hasOwn.call(obj.constructor.prototype, "isPrototypeOf")) {
					return false
				}
			} catch (e) {
				return false
			}
			var key;
			for (key in obj) {}

			return key === undefined || _hasOwn.call(obj, key)
		},
		isEmptyObject : function (obj) {
			var name;
			for (name in obj) {
				return false
			}
			return true
		},
		error : function (msg) {
			throw new Error(msg)
		}
	}, true);
	cQuery.extend("proxy", function () {
		return function (fn, context) {
			var args,
			proxy;
			if (!cQuery.isFunction(fn)) {
				return undefined
			}
			args = _slice.call(arguments, 2);
			var proxy = function () {
				return fn.apply(context, args.concat(_slice.call(arguments)))
			};
			return proxy
		}
	});
	cQuery.extend("util", {
		getUniqueKey : function () {
			return "__cQuery__" + Math.floor(Math.random() * 1000) + new Date().getTime().toString()
		},
		_extend : _extend,
		inherit : function (parent, staticExtend, protoExtend) {
			var child = function () {
				staticExtend && _extend(this, staticExtend);
				parent.apply(this, arguments)
			};
			_inherit(child, parent);
			protoExtend && _extend(child.prototype, protoExtend, true);
			return child
		},
		browser : (function () {
			var b = navigator.userAgent.toLowerCase(),
			c = W.external || "",
			d,
			e,
			f,
			g,
			h,
			i = function (a) {
				var b = 0;
				return parseFloat(a.replace(/\./g, function () {
						return b++ == 1 ? "" : "."
					}))
			};
			try {
				/windows|win32/i.test(b) ? h = "windows" : /macintosh/i.test(b) ? h = "macintosh" : /rhino/i.test(b) && (h = "rhino");
				if ((e = b.match(/applewebkit\/([^\s]*)/)) && e[1]) {
					d = "webkit";
					g = i(e[1])
				} else {
					if ((e = b.match(/presto\/([\d.]*)/)) && e[1]) {
						d = "presto";
						g = i(e[1])
					} else {
						if (e = b.match(/msie\s([^;]*)/)) {
							d = "trident";
							g = 1;
							(e = b.match(/trident\/([\d.]*)/)) && e[1] && (g = i(e[1]))
						} else {
							if (/gecko/.test(b)) {
								d = "gecko";
								g = 1;
								(e = b.match(/rv:([\d.]*)/)) && e[1] && (g = i(e[1]))
							}
						}
					}
				}
				/world/.test(b) ? f = "world" : /360se/.test(b) ? f = "360" : /maxthon/.test(b) || typeof c.max_version == "number" ? f = "maxthon" : /tencenttraveler\s([\d.]*)/.test(b) ? f = "tt" : /se\s([\d.]*)/.test(b) && (f = "sogou")
			} catch (j) {}

			var k = {
				OS : h,
				CORE : d,
				Version : g,
				EXTRA : f ? f : !1,
				IE : /msie/.test(b),
				OPERA : /opera/.test(b),
				MOZ : /gecko/.test(b) && !/(compatible|webkit)/.test(b),
				IE5 : /msie 5 /.test(b),
				IE55 : /msie 5.5/.test(b),
				IE6 : /msie 6/.test(b),
				IE7 : /msie 7/.test(b),
				IE8 : /msie 8/.test(b),
				IE9 : /msie 9/.test(b),
				SAFARI : !/chrome\/([\d.]*)/.test(b) && /\/([\d.]*) safari/.test(b),
				CHROME : /chrome\/([\d.]*)/.test(b),
				IPAD : /\(ipad/i.test(b),
				IPHONE : /\(iphone/i.test(b),
				ITOUCH : /\(itouch/i.test(b),
				MOBILE : /mobile/i.test(b)
			};
			return k
		})(),
		textareautil : (function () {
			var flag = !!document.selection,
			_getPos,
			_setPos,
			_setText,
			_moveTo;
			if (flag) {
				_getPos = function (textarea) {
					textarea.focus();
					var range = {
						text : "",
						start : 0,
						end : 0
					},
					i = 0,
					oS = document.selection.createRange(),
					oR = document.body.createTextRange();
					oR.moveToElementText(textarea);
					range.text = oS.text;
					range.bookmark = oS.getBookmark();
					for (; oR.compareEndPoints("StartToStart", oS) < 0 && oS.moveStart("character", -1) !== 0; i++) {
						if (textarea.value.charAt(i) == "\n") {
							i++
						}
					}
					range.start = i;
					range.end = range.text.length + range.start;
					return range
				};
				_setPos = function (textarea, range) {
					if (!range) {
						this.toHead()
					}
					var oR = textarea.createTextRange();
					LStart = range.start,
					LEnd = range.end,
					start = 0,
					end = 0,
					value = textarea.value;
					for (var i = 0; i < value.length && i < LStart; i++) {
						var c = value.charAt(i);
						if (c != "\n") {
							start++
						}
					}
					for (var i = value.length - 1; i >= LEnd && i >= 0; i--) {
						var c = value.charAt(i);
						if (c != "\n") {
							end++
						}
					}
					oR.moveStart("character", start);
					oR.moveEnd("character", -end);
					oR.select();
					textarea.focus()
				};
				_setText = function (textarea, range, text) {
					var sR;
					this.setPosition(textarea, range);
					sR = document.selection.createRange();
					sR.text = text;
					sR.setEndPoint("StartToEnd", sR);
					sR.select()
				};
				_moveTo = function (textarea, pos) {
					var range = textarea.createTextRange();
					range.collapse(true);
					range.moveEnd("character", pos);
					range.moveStart("character", pos);
					range.select()
				}
			} else {
				_getPos = function (textarea) {
					textarea.focus();
					var range = {
						text : "",
						start : 0,
						end : 0
					};
					range.start = textarea.selectionStart;
					range.end = textarea.selectionEnd;
					range.text = (range.start !== range.end) ? textarea.value.substring(range.start, range.end) : "";
					return range
				};
				_setPos = function (textarea, range) {
					if (!range) {
						this.toHead(textarea);
						return
					}
					textarea.focus();
					textarea.setSelectionRange(range.start, range.end)
				};
				_setText = function (textarea, range, text) {
					var oValue,
					nValue,
					nStart,
					nEnd,
					st;
					this.setPosition(textarea, range);
					oValue = textarea.value;
					nValue = oValue.substring(0, range.start) + text + oValue.substring(range.end);
					nStart = nEnd = range.start + text.length;
					st = textarea.scrollTop;
					textarea.value = nValue;
					if (textarea.scrollTop != st) {
						textarea.scrollTop = st
					}
					textarea.setSelectionRange(nStart, nEnd)
				};
				_moveTo = function (textarea, pos) {
					textarea.focus();
					textarea.setSelectionRange(pos, pos)
				}
			}
			return {
				getPosition : _getPos,
				setPosition : _setPos,
				setText : _setText,
				moveTo : _moveTo,
				toHead : function (textarea) {
					this.moveTo(textarea, 0)
				},
				toTail : function (textarea) {
					this.moveTo(textarea, textarea.value.length)
				}
			}
		})(),
		parseParam : function (oSource, oParams) {
			var key;
			try {
				if (typeof oParams != "undefined") {
					for (key in oSource) {
						if (oParams[key] != null) {
							oSource[key] = oParams[key]
						}
					}
				}
			}
			finally {
				key = null;
				return oSource
			}
		}
	}, true);
	cQuery.extend("util.url", function () {
		Url = function (url) {
			url = url || "";
			this.url = url;
			this.query = {};
			this.parse()
		};
		Url.prototype = {
			parse : function (url) {
				if (url) {
					this.url = url
				}
				this.parseAnchor();
				this.parseParam()
			},
			parseAnchor : function () {
				var anchor = this.url.match(/\#(.*)/);
				anchor = anchor ? anchor[1] : null;
				this._anchor = anchor;
				if (anchor != null) {
					this.anchor = this.getNameValuePair(anchor);
					this.url = this.url.replace(/\#.*/, "")
				}
			},
			parseParam : function () {
				var query = this.url.match(/\?([^\?]*)/);
				query = query ? query[1] : null;
				if (query != null) {
					this.url = this.url.replace(/\?([^\?]*)/, "");
					this.query = this.getNameValuePair(query)
				}
			},
			getNameValuePair : function (str) {
				var o = {};
				str.replace(/([^&=]*)(?:\=([^&]*))?/gim, function (w, n, v) {
					if (n == "") {
						return
					}
					o[n] = v || ""
				});
				return o
			},
			getParam : function (sPara) {
				return this.query[sPara] || ""
			},
			clearParam : function () {
				this.query = {}

			},
			setParam : function (name, value) {
				if (name == null || name == "" || typeof(name) != "string") {
					throw new Error("no param name set")
				}
				this.query = this.query || {};
				this.query[name] = value
			},
			setParams : function (o) {
				this.query = o
			},
			serialize : function (o) {
				var ar = [];
				for (var i in o) {
					if (o[i] == null || o[i] == "") {
						ar.push(i + "=")
					} else {
						ar.push(i + "=" + o[i])
					}
				}
				return ar.join("&")
			},
			toString : function () {
				var queryStr = this.serialize(this.query);
				return this.url + (queryStr.length > 0 ? "?" + queryStr : "") + (this.anchor ? "#" + this.serialize(this.anchor) : "")
			},
			getHashStr : function (forceSharp) {
				return this.anchor ? "#" + this.serialize(this.anchor) : (forceSharp ? "#" : "")
			}
		};
		return Url
	});
	cQuery.extend("str", {
		trim : _trim && !_trim.call("\uFEFF\xA0") ? function (text) {
			return text == null ? "" : _trim.call(text)
		}
		 : function (text) {
			return text == null ? "" : (text + "").replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, "")
		},
		encodeHTML : function (str) {
			if (typeof str != "string") {
				throw "encodeHTML need a string as parameter"
			}
			return str.replace(/\&/g, "&amp;").replace(/"/g, "&quot;").replace(/\</g, "&lt;").replace(/\>/g, "&gt;").replace(/\'/g, "&#39;").replace(/\u00A0/g, "&nbsp;").replace(/(\u0020|\u000B|\u2028|\u2029|\f)/g, "&#32;")
		},
		decodeHTML : function (str) {
			if (typeof str != "string") {
				throw "decodeHTML need a string as parameter"
			}
			return str.replace(/&quot;/g, '"').replace(/&lt;/g, "<").replace(/&gt;/g, ">").replace(/&#39/g, "'").replace(/&nbsp;/g, "?").replace(/&#32;/g, " ").replace(/&amp;/g, "&")
		},
		getByteLength : function (str) {
			if (typeof str == "undefined") {
				return 0
			}
			var aMatch = str.match(/[^\x00-\x80]/g);
			return (str.length + (!aMatch ? 0 : aMatch.length))
		},
		strLeft : function (s, n) {
			var ELLIPSIS = "...";
			var s2 = s.slice(0, n),
			i = s2.replace(/[^\x00-\xff]/g, "**").length,
			j = s.length,
			k = s2.length;
			if (i < n) {
				return s2
			} else {
				if (i == n) {
					if (n == j || k == j) {
						return s2
					} else {
						return s.slice(0, n - 2) + ELLIPSIS
					}
				}
			}
			i -= s2.length;
			switch (i) {
			case 0:
				return s2;
			case n:
				var s4;
				if (n == j) {
					s4 = s.slice(0, (n >> 1) - 1);
					return s4 + ELLIPSIS
				} else {
					s4 = s.slice(0, n >> 1);
					return s4
				}
			default:
				var k = n - i,
				s3 = s.slice(k, n),
				j = s3.replace(/[\x00-\xff]/g, "").length;
				return j ? s.slice(0, k) + arguments.callee(s3, j) : s.slice(0, k)
			}
		}
	}, true);
	cQuery.extend("dom", {
		ready : function (fn, coer) {
			var fns = [],
			inited = 0,
			isReady = 0;
			var checkReady = function () {
				if (document.readyState === "complete") {
					return 1
				}
				return isReady
			};
			var onReady = function (type) {
				if (isReady) {
					return
				}
				isReady = 1;
				if (fns) {
					while (fns.length) {
						fns.shift()()
					}
				}
				fns = null
			};
			var bindReady = function () {
				if (inited) {
					return
				}
				inited = 1;
				if (!coer) {
					if (document.readyState === "complete") {
						onReady()
					} else {
						if (document.addEventListener) {
							document.addEventListener("DOMContentLoaded", function () {
								document.removeEventListener("DOMContentLoaded", arguments.callee, false);
								onReady()
							}, false)
						} else {
							document.attachEvent("onreadystatechange", function () {
								if (document.readyState == "complete") {
									document.detachEvent("onreadystatechange", arguments.callee);
									onReady()
								}
							});
							(function () {
								if (documentElement.doScroll && typeof window.frameElement === "undefined") {
									if (isReady) {
										return
									}
									try {
										documentElement.doScroll("left")
									} catch (e) {
										setTimeout(arguments.callee, 64);
										return
									}
									onReady()
								}
							})()
						}
					}
				}
				cQuery.evt.addEvent(W, "load", onReady)
			};
			bindReady();
			if (!checkReady()) {
				fns.push(fn);
				return
			}
			fn.call()
		},
		GD : function (id) {
			if (!id) {
				return undefined
			} else {
				if (typeof id === "object" && !!id.nodeType) {
					return id
				} else {
					return document.getElementById(id)
				}
			}
		},
		GN : function (tag, elem) {
			elem = elem || document;
			return elem.getElementsByTagName(tag)
		},
		GC : function (clz, el, tg) {
			el = el || document;
			el = cQuery.isString(el) ? cQuery.dom.GD(el) : el;
			tg = tg || "*";
			var rs = [];
			clz = " " + clz + " ";
			var cldr = cQuery.dom.GN(tg, el),
			len = cldr.length;
			for (var i = 0; i < len; ++i) {
				var o = cldr[i];
				if (o.nodeType == 1) {
					var ecl = " " + o.className + " ";
					if (ecl.indexOf(clz) != -1) {
						rs[rs.length] = o
					}
				}
			}
			return rs
		},
		GA : function (attvalue, attname, parentId, tagname) {
			if (typeof parentId === "string" && parentId != "") {
				parentId = document.getElementById(parentId)
			}
			var elms = cQuery.dom.GN((tagname || "*"), (parentId || document)),
			ret = [],
			re = new RegExp("\\b" + attvalue + "\\b");
			for (var i = 0, l = elms.length; i < l; i++) {
				if (elms[i].getAttribute(attname) && re.test(elms[i].getAttribute(attname))) {
					ret.push(elms[i])
				}
			}
			return ret
		},
		hasClass : function (elem, classname) {
			return (new RegExp("(^|\\s)" + classname + "($|\\s)")).test(elem.className)
		},
		addClass : function (elem, classname) {
			elem.nodeType === 1 && (cQuery.dom.hasClass(elem, classname) || (elem.className = cQuery.str.trim(elem.className) + " " + classname))
		},
		removeClass : function (elem, classname) {
			elem.nodeType === 1 && cQuery.dom.hasClass(elem, classname) && (elem.className = elem.className.replace(new RegExp("(^|\\s)" + classname + "($|\\s)"), " "))
		},
		contains : function (root, el) {
			if (root.compareDocumentPosition) {
				return root === el || !!(root.compareDocumentPosition(el) & 16)
			}
			if (root.contains && el.nodeType === 1) {
				return root.contains(el) && root !== el
			}
			while ((el = el.parentNode)) {
				if (el === root) {
					return true
				}
			}
			return false
		},
		getStyle : function (el, property) {
			if (!cQuery.util.browser.IE) {
				if (property == "float") {
					property = "cssFloat"
				}
				try {
					var computed = document.defaultView.getComputedStyle(el, "")
				} catch (e) {}

				return el.style[property] || (computed ? computed[property] : null)
			} else {
				switch (property) {
				case "opacity":
					var val = 100;
					try {
						val = el.filters["DXImageTransform.Microsoft.Alpha"].opacity
					} catch (e) {
						try {
							val = el.filters("alpha").opacity
						} catch (e) {}

					}
					return val / 100;
				case "float":
					property = "styleFloat";
				default:
					var value = el.currentStyle ? el.currentStyle[property] : null;
					return (el.style[property] || value)
				}
			}
		},
		getWinSize : function (win) {
			var w,
			h;
			var target;
			if (win) {
				target = win.document
			} else {
				target = document
			}
			if (self.innerHeight) {
				if (win) {
					target = win.self
				} else {
					target = self
				}
				w = target.innerWidth;
				h = target.innerHeight
			} else {
				if (target.documentElement && target.documentElement.clientHeight) {
					w = target.documentElement.clientWidth;
					h = target.documentElement.clientHeight
				} else {
					if (target.body) {
						w = target.body.clientWidth;
						h = target.body.clientHeight
					}
				}
			}
			return {
				width : w,
				height : h
			}
		},
		getElementPos : function (obj) {
			var isOpera = cQuery.util.browser.OPERA;
			var isIE = (cQuery.util.browser.IE && !isOpera);
			var el = obj;
			if (el.parentNode === null || el.style.display == "none") {
				return false
			}
			var parent = null;
			var pos = [];
			var box;
			if (el.getBoundingClientRect) {
				box = el.getBoundingClientRect();
				var scrollTop = Math.max(document.documentElement.scrollTop, document.body.scrollTop);
				var scrollLeft = Math.max(document.documentElement.scrollLeft, document.body.scrollLeft);
				return {
					x : box.left + scrollLeft,
					y : box.top + scrollTop
				}
			} else {
				if (document.getBoxObjectFor) {
					box = document.getBoxObjectFor(el);
					var borderLeft = (el.style.borderLeftWidth) ? parseInt(el.style.borderLeftWidth) : 0;
					var borderTop = (el.style.borderTopWidth) ? parseInt(el.style.borderTopWidth) : 0;
					pos = [box.x - borderLeft, box.y - borderTop]
				} else {
					pos = [el.offsetLeft, el.offsetTop];
					parent = el.offsetParent;
					if (parent != el) {
						while (parent) {
							pos[0] += parent.offsetLeft;
							pos[1] += parent.offsetTop;
							parent = parent.offsetParent
						}
					}
					if (ua.indexOf("opera") != -1 || (ua.indexOf("safari") != -1 && el.style.position == "absolute")) {
						pos[0] -= document.body.offsetLeft;
						pos[1] -= document.body.offsetTop
					}
				}
			}
			if (el.parentNode) {
				parent = el.parentNode
			} else {
				parent = null
			}
			while (parent && parent.tagName != "BODY" && parent.tagName != "HTML") {
				pos[0] -= parent.scrollLeft;
				pos[1] -= parent.scrollTop;
				if (parent.parentNode) {
					parent = parent.parentNode
				} else {
					parent = null
				}
			}
			return {
				x : pos[0],
				y : pos[1]
			}
		},
		getMousePos : function (event) {
			var event = event || window.event;
			var sLeft = documentElement.scrollLeft || document.body.scrollLeft;
			var sTop = documentElement.scrollTop || document.body.scrollTop;
			var mouseCoords = function (event) {
				if (event.pageX || event.pageY) {
					return {
						x : event.pageX,
						y : event.pageY
					}
				}
				return {
					x : event.clientX + sLeft - document.body.clientLeft,
					y : event.clientY + sTop - document.body.clientTop
				}
			};
			var mousePos = mouseCoords(event);
			return mousePos
		},
		insertAfter : function (ele, rEle) {
			try {
				var par = rEle.parentNode;
				par.lastChild == rEle ? par.appendChild(ele) : par.insertBefore(ele, rEle.nextSibling);
				return "successed"
			} catch (e) {
				return "failed"
			}
		},
		insertBefore : function (ele, rEle) {
			try {
				var par = rEle.parentNode;
				par.insertBefore(ele, rEle);
				return "successed"
			} catch (e) {
				return "failed"
			}
		},
		parseDOM : function (arg) {
			var objE = document.createElement("DIV");
			objE.innerHTML = arg;
			return objE.childNodes
		},
		divCreator : function (str) {
			var frag = document.createElement("div");
			frag.innerHTML = str;
			return frag
		},
		createFragment : function (nodes) {
			if (nodes && nodes.length) {
				var frag = null,
				i = 0,
				length = nodes.length;
				document = document || nodes[0].ownerDocument;
				frag = document.createDocumentFragment();
				if (nodes.item) {
					nodes = cQuery.arr.arrayify(nodes)
				}
				for (i = 0, len = nodes.length; i < len; i++) {
					frag.appendChild(nodes[i])
				}
				return frag
			}
			return null
		},
		createFromStr : function (str) {
			var trim = cQuery.str.trim,
			divCreator = cQuery.dom.divCreator,
			createFragment = cQuery.dom.createFragment;
			if (typeof str === "string" && (str = trim(str))) {
				var nodes = null,
				created,
				creator = divCreator;
				created = creator(str).childNodes;
				if (created.length === 1) {
					nodes = created[0].parentNode.removeChild(created[0])
				} else {
					nodes = createFragment(created)
				}
				return nodes
			}
			return null
		}
	}, true);
	cQuery.extend("arr", {
		indexOf : function (tar, arr) {
			if (arr.indexOf) {
				return arr.indexOf(tar)
			}
			var i = 0,
			len = arr.length;
			while (i < len) {
				if (arr[i] === tar) {
					return i
				}
				i++
			}
			return -1
		},
		inArray : function (tar, arr) {
			return cQuery.arr.indexOf(tar, arr) > -1
		},
		arrayify : function (obj) {
			if (obj === undefined) {
				return []
			}
			if (cQuery.isArray(obj)) {
				return obj
			}
			if (obj === null || typeof obj.length !== "number" || cQuery.isString(obj) || cQuery.isFunction(obj)) {
				return [obj]
			}
			return Array.prototype.slice.apply(obj)
		}
	}, true);
	cQuery.extend("json", {
		jsonToQuery : function (JSON, isEncode) {
			var _Qstring = [];
			var _fdata = function (data, isEncode) {
				data = data == null ? "" : data;
				data = cQuery.str.trim(data.toString());
				if (isEncode) {
					return encodeURIComponent(data)
				} else {
					return data
				}
			};
			if (typeof JSON == "object") {
				for (var k in JSON) {
					if (JSON[k]instanceof Array) {
						for (var i = 0, len = JSON[k].length; i < len; i++) {
							_Qstring.push(k + "=" + _fdata(JSON[k][i], isEncode))
						}
					} else {
						if (typeof JSON[k] != "function") {
							_Qstring.push(k + "=" + _fdata(JSON[k], isEncode))
						}
					}
				}
			}
			if (_Qstring.length) {
				return _Qstring.join("&")
			} else {
				return ""
			}
		},
		queryToJson : function (QS, isDecode) {
			var _Qlist = cQuery.str.trim(QS).split("&");
			var _json = {};
			var _fData = function (data) {
				if (isDecode) {
					return decodeURIComponent(data)
				} else {
					return data
				}
			};
			for (var i = 0, len = _Qlist.length; i < len; i++) {
				if (_Qlist[i]) {
					_hsh = _Qlist[i].split("=");
					_key = _hsh[0];
					_value = _hsh[1];
					if (_hsh.length < 2) {
						_value = _key;
						_key = "$nullName"
					}
					if (!_json[_key]) {
						_json[_key] = _fData(_value)
					} else {
						if (cQuery.isArray(_json[_key]) != true) {
							_json[_key] = [_json[_key]]
						}
						_json[_key].push(_fData(_value))
					}
				}
			}
			return _json
		}
	}, true);
	cQuery.extend("evt", {
		addEvent : function (obj, evt, func, eventobj) {
			var eventobj = !eventobj ? obj : eventobj,
			evt = evt || "click",
			func = func || cQuery.noop;
			if (obj.addEventListener) {
				obj.addEventListener(evt, func, false)
			} else {
				if (eventobj.attachEvent) {
					obj.attachEvent("on" + evt, func)
				}
			}
		},
		removeEvent : function (obj, evt, func, eventobj) {
			var eventobj = !eventobj ? obj : eventobj,
			evt = evt || "click",
			func = func || cQuery.noop;
			if (obj.removeEventListener) {
				obj.removeEventListener(evt, func, false)
			} else {
				if (eventobj.detachEvent) {
					obj.detachEvent("on" + evt, func)
				}
			}
		},
		fixEvent : function (e) {
			if (typeof e == "undefined") {
				e = window.event
			}
			if (!e.target) {
				e.target = e.srcElement;
				e.pageX = e.x;
				e.pageY = e.y
			}
			if (typeof e.layerX == "undefined") {
				e.layerX = e.offsetX
			}
			if (typeof e.layerY == "undefined") {
				e.layerY = e.offsetY
			}
			return e
		},
		preventDefault : function (e) {
			var e = e || window.event;
			try {
				e.preventDefault()
			} catch (e) {
				e.returnValue = false
			}
		},
		custEvent : (function () {
			var _custAttr = "__custEventKey__",
			_custKey = 1,
			_custCache = {},
			_findObj = function (obj, type) {
				var _key = (typeof obj == "number") ? obj : obj[_custAttr];
				return (_key && _custCache[_key]) && {
					obj : (typeof type == "string" ? _custCache[_key][type] : _custCache[_key]),
					key : _key
				}
			};
			return {
				define : function (obj, type) {
					if (obj && type) {
						var _key = (typeof obj == "number") ? obj : obj[_custAttr] || (obj[_custAttr] = _custKey++),
						_cache = _custCache[_key] || (_custCache[_key] = {});
						type = [].concat(type);
						for (var i = 0; i < type.length; i++) {
							_cache[type[i]] || (_cache[type[i]] = [])
						}
						return _key
					}
				},
				undefine : function (obj, type) {
					if (obj) {
						var _key = (typeof obj == "number") ? obj : obj[_custAttr];
						if (_key && _custCache[_key]) {
							if (typeof type == "string") {
								if (type in _custCache[_key]) {
									delete _custCache[_key][type]
								}
							} else {
								delete _custCache[_key]
							}
						}
					}
				},
				add : function (obj, type, fn, data) {
					if (obj && typeof type == "string" && fn) {
						var _cache = _findObj(obj, type);
						if (!_cache || !_cache.obj) {
							throw "custEvent (" + type + ") is undefined !"
						}
						_cache.obj.push({
							fn : fn,
							data : data
						});
						return _cache.key
					}
				},
				remove : function (obj, type, fn) {
					if (obj) {
						var _cache = _findObj(obj, type),
						_obj;
						if (_cache && (_obj = _cache.obj)) {
							if (cQuery.isArray(_obj)) {
								if (fn) {
									for (var i = 0; i < _obj.length && _obj[i].fn !== fn; i++) {}

									_obj.splice(i, 1)
								} else {
									_obj.splice(0)
								}
							} else {
								for (var i in _obj) {
									_obj[i] = []
								}
							}
							return _cache.key
						}
					}
				},
				fire : function (obj, type, args) {
					if (obj && typeof type == "string") {
						var _cache = _findObj(obj, type),
						_obj;
						if (_cache && (_obj = _cache.obj)) {
							if (!cQuery.isArray(args)) {
								args = args != undefined ? [args] : []
							}
							if (_obj.length > 0) {
								(function () {
									var start = +new Date();
									do {
										var item = _obj.shift();
										var fn = item.fn;
										var data = item.data;
										var context = data && data.context;
										if (fn && fn.apply) {
											fn.apply(context || cQuery, [{
														type : type,
														data : data
													}
												].concat(args))
										}
									} while (_obj.length > 0 && (+new Date() - start < 50));
									if (_obj.length > 0) {
										timeId = setTimeout(arguments.callee, 25)
									}
								})()
							}
							return _cache.key
						}
					}
				},
				destroy : function () {
					_custCache = {};
					_custKey = 1
				}
			}
		})(),
		delegatedEvent : function (actEl, expEls, aType) {
			var checkContains = function (list, el) {
				for (var i = 0, len = list.length; i < len; i += 1) {
					if (cQuery.dom.contains(list[i], el)) {
						return true
					}
				}
				return false
			};
			var isNode = function (oNode) {
				return !!((oNode != undefined) && oNode.nodeName && oNode.nodeType)
			};
			if (!isNode(actEl)) {
				throw "delegatedEvent need an Element as first Parameter"
			}
			if (!expEls) {
				expEls = []
			}
			if (cQuery.isArray(expEls)) {
				expEls = [expEls]
			}
			var evtList = {};
			var aType = aType || "action-type";
			var bindEvent = function (e) {
				var evt = cQuery.evt.fixEvent(e);
				var el = evt.target;
				var type = e.type;
				if (checkContains(expEls, el)) {
					return false
				} else {
					if (!cQuery.dom.contains(actEl, el)) {
						return false
					} else {
						var actionType = null;
						var checkBuble = function () {
							if (evtList[type] && evtList[type][actionType]) {
								return evtList[type][actionType]({
									evt : evt,
									el : el,
									e : e,
									data : cQuery.json.queryToJson(el.getAttribute("action-data") || "")
								})
							} else {
								return true
							}
						};
						while (el && el !== actEl) {
							if (!el.getAttribute) {
								break
							}
							actionType = el.getAttribute(aType);
							if (checkBuble() === false) {
								break
							}
							el = el.parentNode
						}
					}
				}
			};
			var that = {};
			that.add = function (funcName, evtType, process) {
				if (!evtList[evtType]) {
					evtList[evtType] = {};
					cQuery.evt.addEvent(actEl, evtType, bindEvent)
				}
				var ns = evtList[evtType];
				ns[funcName] = process
			};
			that.remove = function (funcName, evtType) {
				if (evtList[evtType]) {
					delete evtList[evtType][funcName];
					if (cQuery.isEmptyObject(evtList[evtType])) {
						delete evtList[evtType];
						cQuery.evt.removeEvent(actEl, bindEvent, evtType)
					}
				}
			};
			that.pushExcept = function (el) {
				expEls.push(el)
			};
			that.removeExcept = function (el) {
				if (!el) {
					expEls = []
				} else {
					for (var i = 0, len = expEls.length; i < len; i += 1) {
						if (expEls[i] === el) {
							expEls.splice(i, 1)
						}
					}
				}
			};
			that.clearExcept = function (el) {
				expEls = []
			};
			that.destroy = function () {
				for (k in evtList) {
					for (l in evtList[k]) {
						delete evtList[k][l]
					}
					delete evtList[k];
					cQuery.evt.removeEvent(actEl, bindEvent, k)
				}
			};
			return that
		}
	}, true);
	cQuery.extend("io", {
		jsonp : function (url, params, callback, callbackname, fix) {
			var byId = cQuery.dom.GD;
			var idStr = url + "&" + params;
			var callbackname = callbackname || "callback";
			var fun = "";
			if (byId(url)) {
				document.body.removeChild(byId(url))
			}
			fix = fix || false;
			if (!fix) {
				url = url + ((url.indexOf("?") == -1) ? "?" : "&") + "_t=" + Math.random();
				if (typeof callback == "function") {
					fun = "fun_" + new Date().getUTCMilliseconds() + ("" + Math.random()).substring(3);
					eval(fun + "=function(res){callback(res)}")
				}
			} else {
				url = url + ((url.indexOf("?") == -1) ? "?" : "&") + "_t=fixed";
				if (typeof callback == "string") {
					fun = callback
				}
			}
			url = url + "&" + callbackname + "=" + fun;
			url = url + "&" + params;
			var head_dom = cQuery.dom.GN("head")[0];
			var old_script = byId(idStr);
			if (old_script) {
				head_dom.removeChild(old_script)
			}
			var script_dom = document.createElement("script");
			script_dom.src = url;
			script_dom.id = idStr;
			script_dom.type = "text/javascript";
			script_dom.language = "javascript";
			head_dom.appendChild(script_dom)
		},
		ajax : function () {
			return {
				createRequest : function () {
					var request = null;
					try {
						request = new XMLHttpRequest()
					} catch (trymicrosoft) {
						try {
							request = new ActiveXObject("Msxml2.XMLHTTP")
						} catch (othermicrosoft) {
							try {
								request = ActiveXObject("Microsoft.XMLHTTP")
							} catch (failed) {}

						}
					}
					if (request == null) {
						throw("<b>create request failed</b>", {
							html : true
						})
					} else {
						return request
					}
				},
				request : function (url, option) {
					option = option || {};
					option.onComplete = option.onComplete || function () {};
					option.onException = option.onException || function () {};
					option.onTimeout = option.onTimeout || function () {};
					option.timeout = option.timeout ? option.timeout : -1;
					option.returnType = option.returnType || "txt";
					option.method = option.method || "get";
					option.data = option.data || {};
					if (typeof option.GET != "undefined" && typeof option.GET.url_random != "undefined" && option.GET.url_random == 0) {
						this.rand = false;
						option.GET.url_random = null
					}
					this.loadData(url, option)
				},
				loadData : function (url, option) {
					var request = this.createRequest(),
					tmpArr = [];
					var _url = new cQuery.util.url(url);
					var timer;
					if (option.POST) {
						for (var postkey in option.POST) {
							var postvalue = option.POST[postkey];
							if (postvalue != null) {
								tmpArr.push(postkey + "=" + encodeURIComponent(postvalue))
							}
						}
					}
					var sParameter = tmpArr.join("&") || "";
					if (option.GET) {
						for (var key in option.GET) {
							if (key != "url_random") {
								_url.setParam(key, encodeURIComponent(option.GET[key]))
							}
						}
					}
					if (this.rand != false) {
						_url.setParam("rnd", Math.random())
					}
					if (option.timeout > -1) {
						timer = setTimeout(option.onTimeout, option.timeout)
					}
					request.onreadystatechange = function () {
						if (request.readyState == 4) {
							var response,
							type = option.returnType;
							try {
								switch (type) {
								case "txt":
									response = request.responseText;
									break;
								case "xml":
									if (cQuery.util.browser.IE) {
										response = request.responseXML
									} else {
										var Dparser = new DOMParser();
										response = Dparser.parseFromString(request.responseText, "text/xml")
									}
									break;
								case "json":
									response = eval("(" + request.responseText + ")");
									break
								}
								option.onComplete(response);
								clearTimeout(timer)
							} catch (e) {
								option.onException(e.message, _url);
								return false
							}
						}
					};
					try {
						if (option.POST) {
							request.open("POST", _url, true);
							request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
							request.send(sParameter)
						} else {
							request.open("GET", _url, true);
							request.send(null)
						}
					} catch (e) {
						option.onException(e.message, _url);
						return false
					}
				}
			}
		},
		ijax : (function () {
			Function.prototype.bind2 = function (object) {
				var __method = this;
				return function () {
					return __method.apply(object, arguments)
				}
			};
			var bind2 = function (fFunc, object) {
				var __method = fFunc;
				return function () {
					return __method.apply(object, arguments)
				}
			};
			return {
				arrTaskLists : [],
				createLoadingIframe : function () {
					if (this.loadFrames != null) {
						return false
					}
					var rndId1 = "loadingIframe_thread" + Math.ceil(Math.random() * 10000);
					var rndId2 = "loadingIframe_thread" + Math.ceil((Math.random() + 1) * 10000);
					this.loadFrames = [rndId1, rndId2];
					var iframeSrc = "";
					if (cQuery.util.browser.IE6) {
						iframeSrc = "javascript:void((function(){document.open();document.domain='sina.com.cn';document.close()})())"
					}
					var html = '<iframe id="' + rndId1 + '" name="' + rndId1 + '" class="invisible" scrolling="no" src="" allowTransparency="true" style="display:none;" frameborder="0"></iframe><iframe id="' + rndId2 + '" name="' + rndId2 + '" class="invisible" scrolling="no" src="' + iframeSrc + '" allowTransparency="true" style="display:none;" frameborder="0"></iframe>';
					var oIjaxIframeCnt = document.createElement("div");
					oIjaxIframeCnt.id = "ijax_iframes";
					oIjaxIframeCnt.innerHTML = html;
					document.body.appendChild(oIjaxIframeCnt);
					var loadTimer = setInterval(bind2(function () {
								if (cQuery.dom.GD(this.loadFrames[0]) != null && cQuery.dom.GD(this.loadFrames[1]) != null) {
									clearInterval(loadTimer);
									loadTimer = null;
									this.loadingIframe = {
										thread1 : {
											container : cQuery.dom.GD(this.loadFrames[0]),
											isBusy : false
										},
										thread2 : {
											container : cQuery.dom.GD(this.loadFrames[1]),
											isBusy : false
										}
									};
									this.loadByList()
								}
							}, this), 10)
				},
				isIjaxReady : function () {
					if (typeof this.loadingIframe == "undefined") {
						return false
					}
					for (var oLoadCnt in this.loadingIframe) {
						if (this.loadingIframe[oLoadCnt].isBusy == false) {
							this.loadingIframe[oLoadCnt].isBusy = true;
							return this.loadingIframe[oLoadCnt]
						}
					}
					return false
				},
				request : function (url, option) {
					var oTask = {};
					oTask.url = url;
					oTask.option = option || {};
					this.arrTaskLists.push(oTask);
					if (this.loadFrames == null) {
						this.createLoadingIframe()
					} else {
						this.loadByList()
					}
				},
				loadByList : function () {
					if (this.arrTaskLists.length == 0) {
						return false
					}
					var loadStatus = this.isIjaxReady();
					if (loadStatus == false) {
						return false
					}
					var newData = this.arrTaskLists[0];
					this.loadData(newData.url, newData.option, loadStatus);
					this.arrTaskLists.shift()
				},
				loadData : function (url, option, loader) {
					var _url = new cQuery.util.url(url);
					if (option.GET) {
						for (var key in option.GET) {
							_url.setParam(key, encodeURIComponent(option.GET[key]))
						}
					}
					_url = _url.toString();
					var ifm = loader.container;
					ifm.listener = bind2(function () {
							if (option.onComplete || option.onException) {
								try {
									var iframeObject = ifm.contentWindow.document,
									sResult;
									var tArea = cQuery.dom.GN("textarea", iframeObject)[0];
									if (typeof tArea != "undefined") {
										sResult = tArea.value
									} else {
										sResult = iframeObject.body.innerHTML
									}
									if (option.onComplete) {
										option.onComplete(sResult)
									} else {
										option.onException()
									}
								} catch (e) {
									if (option.onException) {
										option.onException(e.message, _url.toString())
									}
								}
							}
							loader.isBusy = false;
							cQuery.evt.removeEvent(ifm, "load", ifm.listener);
							this.loadByList()
						}, this);
					cQuery.evt.addEvent(ifm, "load", ifm.listener);
					if (option.POST) {
						var oIjaxForm = document.createElement("form");
						oIjaxForm.id = "IjaxForm";
						oIjaxForm.action = _url;
						oIjaxForm.method = "post";
						oIjaxForm.target = ifm.id;
						for (var oItem in option.POST) {
							var oInput = document.createElement("input");
							oInput.type = "hidden";
							oInput.name = oItem;
							oInput.value = option.POST[oItem];
							oIjaxForm.appendChild(oInput)
						}
						document.body.appendChild(oIjaxForm);
						try {
							oIjaxForm.submit()
						} catch (e) {}

					} else {
						try {
							window.frames(ifm.id).location.href = _url
						} catch (e) {
							ifm.src = _url
						}
					}
				}
			}
		})(),
		jsLoader : (function () {
			var JsLoad = {};
			(function () {
				function createScripts(oOpts, oCFG) {
					processUrl(oOpts, oCFG);
					var urls = oOpts.urls;
					var i,
					len = urls.length;
					for (i = 0; i < len; i++) {
						var js = document.createElement("script");
						js.src = urls[i].url;
						js[document.all ? "onreadystatechange" : "onload"] = function () {
							if (/gecko/.test(navigator.userAgent.toLowerCase()) || this.readyState.toLowerCase() == "complete" || this.readyState.toLowerCase() == "loaded") {
								oCFG.script_loaded_num++
							}
						};
						cQuery.dom.GN("head")[0].appendChild(js)
					}
				}
				function processUrl(oOpts, oCFG) {
					var urls = oOpts.urls;
					var get_hash = oOpts.GET;
					var i,
					len = urls.length;
					var key,
					url_cls,
					jsvar,
					jsvar,
					rnd;
					for (i = 0; i < len; i++) {
						rnd = parseInt(Math.random() * 100000000);
						url_cls = new cQuery.util.url(urls[i].url);
						jsvar = url_cls.getParam("jsvar") || "requestId_" + rnd;
						if (oOpts.noreturn != true) {
							url_cls.setParam("jsvar", jsvar)
						}
						oCFG.script_var_arr.push(jsvar);
						for (key in get_hash) {
							if (oOpts.noencode == true) {
								url_cls.setParam(key, get_hash[key])
							} else {
								url_cls.setParam(key, encodeURIComponent(get_hash[key]))
							}
						}
						urls[i].url = url_cls.toString();
						urls[i].charset = urls[i].charset || oOpts.charset
					}
				}
				function ancestor(aUrls, oOpts) {
					var _opts = {
						urls : [],
						charset : "utf-8",
						noreturn : false,
						noencode : true,
						timeout : -1,
						POST : {},
						GET : {},
						onComplete : null,
						onException : null
					};
					var _cfg = {
						script_loaded_num : 0,
						is_timeout : false,
						is_loadcomplete : false,
						script_var_arr : []
					};
					_opts.urls = typeof aUrls == "string" ? [{
								url : aUrls
							}
						] : aUrls;
					cQuery.util.parseParam(_opts, oOpts);
					createScripts(_opts, _cfg);
					(function () {
						if (_opts.noreturn == true && _opts.onComplete == null) {
							return
						}
						var i,
						data = [];
						if (_cfg.script_loaded_num == _opts.urls.length) {
							_cfg.is_loadcomplete = true;
							if (_opts.onComplete != null) {
								for (i = 0; i < _cfg.script_var_arr.length; i++) {
									data.push(window[_cfg.script_var_arr[i]])
								}
								if (_cfg.script_var_arr.length < 2) {
									_opts.onComplete(data[0])
								} else {
									_opts.onComplete(data)
								}
							}
							return
						}
						if (_cfg.is_timeout == true) {
							return
						}
						setTimeout(arguments.callee, 50)
					})();
					if (_opts.timeout > 0) {
						setTimeout(function () {
							if (_cfg.is_loadcomplete != true) {
								if (_opts.onException != null) {
									_opts.onException()
								}
								_cfg.is_timeout = true
							}
						}, _opts.timeout)
					}
				}
				JsLoad.request = function (aUrls, oOpts) {
					new ancestor(aUrls, oOpts)
				}
			})();
			return JsLoad
		})(),
		html5Ijax : (function () {
			var _add = cQuery.evt.addEvent,
			_remove = cQuery.evt.removeEvent,
			NOOP = function () {},
			RE_URL = /^http\s?\:\/\/[a-z\d\-\.]+/i,
			ID_PREFIX = "ijax-html5-iframe-",
			MsgSender = function (cfg) {
				cfg = cfg || {};
				this.init(cfg)
			};
			MsgSender.prototype = {
				ready : false,
				init : function (cfg) {
					if (this.ready) {
						return
					}
					var self = this,
					iframeId,
					iframeHtml,
					iframe,
					loaded,
					receiver,
					proxyUrl = cfg.proxyUrl,
					datas = {};
					self.onsuccess = cfg.onsuccess || NOOP;
					self.onfailure = cfg.onfailure || NOOP;
					if (!proxyUrl) {
						return
					}
					receiver = function (e) {
						if (!self.ready || e.origin !== self.target) {
							self.destroy();
							return
						}
						var ret = e.data;
						if (!ret || ret === "failure") {
							self.destroy();
							self.onfailure && self.onfailure()
						} else {
							self.onsuccess && self.onsuccess(e.data);
							self.destroy()
						}
					};
					_add(window, "message", receiver);
					iframeId = ID_PREFIX + Date.parse(new Date());
					iframeHtml = '<iframe id="' + iframeId + '" name="' + iframeId + '" src="' + proxyUrl + '" frameborder="0" style="width:0;height:0;display:none;"></iframe>';
					var oIjaxIframeCnt = document.createElement("div");
					oIjaxIframeCnt.id = ID_PREFIX + "iframes";
					oIjaxIframeCnt.innerHTML = iframeHtml;
					iframe = oIjaxIframeCnt.childNodes[0];
					loaded = function () {
						self.ready = true;
						var src = iframe.src,
						matched = src.match(RE_URL);
						self.target = (matched && matched[0]) || "*"
					};
					_add(iframe, "load", loaded);
					document.body.insertBefore(iframe, document.body.firstChild);
					self._iframe = iframe;
					self._iframeLoaded = loaded;
					self._receiver = receiver
				},
				send : function (cfg) {
					cfg = cfg || {};
					var self = this,
					url = cfg.url,
					data = cfg.data,
					onsuccess = cfg.onsuccess,
					onfailure = cfg.onfailure;
					if (!url || typeof url !== "string") {
						return
					}
					if (onsuccess) {
						self.onsuccess = onsuccess
					}
					if (onfailure) {
						self.onfailure = onfailure
					}
					if (!self.ready) {
						setTimeout(function () {
							self.send(cfg)
						}, 50);
						return
					}
					if (data) {
						data += "&_url=" + window.encodeURIComponent(url)
					} else {
						data = "_url=" + window.encodeURIComponent(url)
					}
					self._iframe.contentWindow.postMessage(data, self.target)
				},
				destroy : function () {
					var iframe = this._iframe;
					_remove(iframe, "load", this._iframeLoaded);
					iframe.parentNode.removeChild(iframe);
					_remove(window, "message", this._receiver);
					this._iframe = null;
					this._iframeLoaded = null;
					this._receiver = null
				}
			};
			return MsgSender
		})()
	}, true);
	cQuery.noConflict = function () {
		if (W.cQuery === cQuery) {
			W.cQuery = _cQuery
		}
		return cQuery
	};
	W.cQuery = cQuery
})(window);
cQuery.extend("app.pageConstructor", function () {
	var e = window;
	var d = e.document;
	var a = cQuery.dom.GN("BODY", d)[0];
	var b = cQuery.evt.addEvent;
	var c = function (f) {
		this.wrap = f.wrap || null;
		this.pagelen = f.pagelen || 20;
		this.count = f.count || 20;
		this.onPageChange = f.onPageChange || undefined;
		this.pagenum = f.pagenum
	};
	c.prototype = {
		_init : function () {
			var h = this;
			var g = h.count;
			h.curpage = 1;
			var f = h._DOMconstruct(h.curpage, g);
			h._eachWrapFun(function () {
				var i = arguments[0];
				i.innerHTML = f
			});
			h._eachWrapFun(function () {
				var i = arguments[0];
				b(i, "click", function (n) {
					var n = n || e.event;
					var m = n.target || n.srcElement;
					if (m.tagName === "A") {
						h.curpage = +m.getAttribute("page");
						var j = h._DOMconstruct(h.curpage, g);
						h._eachWrapFun(function () {
							var o = arguments[0];
							o.innerHTML = j;
							cQuery.isFunction(h.onPageChange) && h.onPageChange.apply(h, [h.curpage, h.pagelen])
						})
					}
				})
			})
		},
		_eachWrapFun : function (j) {
			var h = this;
			if (!cQuery.isFunction(j)) {
				return
			}
			for (var g = 0, f = h.wrap.length; g < f; g++) {
				j.call(h, h.wrap[g])
			}
		},
		_DOMconstruct : function (q, m) {
			var p = this;
			var m = +m || p.count;
			var h = Math.ceil(m / p.pagelen);
			var o = [];
			if (h <= 1) {
				o = ["<span>\u6CA1\u6709\u66F4\u591A\u6587\u7AE0\u4E86</span>"]
			} else {
				if (q <= Math.ceil(p.pagenum / 2)) {
					if (q === 1) {
						o.push("<span>&lt;&lt;</span>")
					} else {
						o.push('<a href="javascript:;" page="' + (q - 1) + '">&lt;&lt;</a>')
					}
					var n = (p.pagenum > h ? h : p.pagenum);
					for (var g = 0; g < n; g++) {
						if (q === g + 1) {
							o.push('<span class="cur">' + (g + 1) + "</span>")
						} else {
							o.push('<a href="javascript:;" page="' + (g + 1) + '">' + (g + 1) + "</a>")
						}
					}
					if (h > p.pagenum) {
						o.push('...<a href="javascript:;" page="' + h + '">' + h + "</a>")
					}
					if (q === h) {
						o.push("<span>&gt;&gt;</span>")
					} else {
						o.push('<a href="javascript:;" page="' + (q + 1) + '">&gt;&gt;</a>')
					}
				} else {
					if (q > h - Math.ceil(p.pagenum / 2)) {
						o.push('<a href="javascript:;" page="' + (q - 1) + '">&lt;&lt;</a>');
						if (h > p.pagenum) {
							o.push('<a href="javascript:;" page="1">1</a>...')
						}
						for (var g = Math.max(h - p.pagenum, 0); g < h; g++) {
							if (q === g + 1) {
								o.push('<span class="cur">' + (g + 1) + "</span>")
							} else {
								o.push('<a href="javascript:;" page="' + (g + 1) + '">' + (g + 1) + "</a>")
							}
						}
						if (q === h) {
							o.push("<span>&gt;&gt;</span>")
						} else {
							o.push('<a href="javascript:;" page="' + (q + 1) + '">&gt;&gt;</a>')
						}
					} else {
						o.push('<a href="javascript:;" page="' + (q - 1) + '">&lt;&lt;</a>');
						o.push('<a href="javascript:;" page="1">1</a>...');
						for (var g = q - Math.ceil(p.pagenum / 2), f = 0; f < p.pagenum; g++, f++) {
							if (q === g + 1) {
								o.push('<span class="cur">' + (g + 1) + "</span>")
							} else {
								o.push('<a href="javascript:;" page="' + (g + 1) + '">' + (g + 1) + "</a>")
							}
						}
						o.push('...<a href="javascript:;" page="' + h + '">' + h + "</a>");
						o.push('<a href="javascript:;" page="' + (q + 1) + '">&gt;&gt;</a>')
					}
				}
			}
			return o.join("")
		}
	};
	return c
});
