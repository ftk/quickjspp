This is a version of QuickJS from https://github.com/c-smile/quickjspp with the following patch applied (fixes C++17 compilation):

```diff
diff --git a/quickjs.h b/quickjs.h
index abd700f..3d36037 100644
--- a/quickjs.h
+++ b/quickjs.h
@@ -328,12 +328,21 @@ typedef struct JSValue {
 #define JS_VALUE_GET_FLOAT64(v) ((v).u.float64)
 #define JS_VALUE_GET_PTR(v) ((v).u.ptr)

+#ifdef __cplusplus
+#define JS_MKVAL(tag, val) JSValue{ JSValueUnion{ .int32 = val }, tag }
+#define JS_MKPTR(tag, p) JSValue{ JSValueUnion{ .ptr = p }, tag }
+
+#define JS_TAG_IS_FLOAT64(tag) ((unsigned)(tag) == JS_TAG_FLOAT64)
+
+#define JS_NAN JSValue{ .u.float64 = JS_FLOAT64_NAN, JS_TAG_FLOAT64 }
+#else // __cplusplus
 #define JS_MKVAL(tag, val) (JSValue){ (JSValueUnion){ .int32 = val }, tag }
 #define JS_MKPTR(tag, p) (JSValue){ (JSValueUnion){ .ptr = p }, tag }

 #define JS_TAG_IS_FLOAT64(tag) ((unsigned)(tag) == JS_TAG_FLOAT64)

 #define JS_NAN (JSValue){ .u.float64 = JS_FLOAT64_NAN, JS_TAG_FLOAT64 }
+#endif // __cplusplus

 static inline JSValue __JS_NewFloat64(JSContext *ctx, double d)
 {
```

# QuickJS Javascript Engine 

Authors: Fabrice Bellard and Charlie Gordon

Ported from https://bellard.org/quickjs/ and its official GitHub mirror https://github.com/bellard/quickjs

By Andrew Fedoniouk (a.k.a. c-smile)

This version is 

* Microsoft Visual C++ compatible/compileable
* Is used in Sciter.JS
* It contains extras, check [wiki](https://github.com/c-smile/quickjspp/wiki) 

The main documentation is in doc/quickjs.pdf or doc/quickjs.html.



