/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef jsstrinlines_h___
#define jsstrinlines_h___

#include "jsatom.h"
#include "jsstr.h"

#include "jscntxtinlines.h"
#include "jsgcinlines.h"

namespace js {

static inline bool
CheckStringLength(JSContext *cx, size_t length)
{
    if (JS_UNLIKELY(length > JSString::MAX_LENGTH)) {
        if (JS_ON_TRACE(cx)) {
            /*
             * If we can't leave the trace, signal OOM condition, otherwise
             * exit from trace before throwing.
             */
            if (!CanLeaveTrace(cx))
                return NULL;

            LeaveTrace(cx);
        }
        js_ReportAllocationOverflow(cx);
        return false;
    }

    return true;
}

/*
 * String builder that eagerly checks for over-allocation past the maximum
 * string length.
 *
 * Note: over-allocation is not checked for when using the infallible
 * |replaceRawBuffer|, so the implementation of |finishString| also must check
 * for over-allocation.
 */
class StringBuffer
{
    typedef Vector<jschar, 32> CharBuffer;
    CharBuffer cb;

    static inline bool checkLength(JSContext *cx, size_t length);
    inline bool checkLength(size_t length);
    JSContext *context() const { return cb.allocPolicy().context(); }
    jschar *extractWellSized();

  public:
    explicit inline StringBuffer(JSContext *cx);
    bool reserve(size_t len);
    bool resize(size_t len);
    bool append(const jschar c);
    bool append(const jschar *chars, size_t len);
    bool append(const jschar *begin, const jschar *end);
    bool append(JSString *str);
    bool append(JSAtom *atom);
    bool appendN(const jschar c, size_t n);
    bool appendInflated(const char *cstr, size_t len);

    /* Infallible variants usable when the corresponding space is reserved. */
    void infallibleAppend(const jschar c) {
        cb.infallibleAppend(c);
    }
    void infallibleAppend(const jschar *chars, size_t len) {
        cb.infallibleAppend(chars, len);
    }
    void infallibleAppend(const jschar *begin, const jschar *end) {
        cb.infallibleAppend(begin, end);
    }
    void infallibleAppendN(const jschar c, size_t n) {
        cb.infallibleAppendN(c, n);
    }

    JSAtom *atomize(uintN flags = 0);
    static JSAtom *atomize(JSContext *cx, const CharBuffer &cb, uintN flags = 0);
    static JSAtom *atomize(JSContext *cx, const jschar *begin, size_t length, uintN flags = 0);

    void replaceRawBuffer(jschar *chars, size_t len) { cb.replaceRawBuffer(chars, len); }
    jschar *begin() { return cb.begin(); }
    jschar *end() { return cb.end(); }
    const jschar *begin() const { return cb.begin(); }
    const jschar *end() const { return cb.end(); }
    bool empty() const { return cb.empty(); }
    inline jsint length() const;

    /*
     * Creates a string from the characters in this buffer, then (regardless
     * whether string creation succeeded or failed) empties the buffer.
     */
    JSFixedString *finishString();

    /* Identical to finishString() except that an atom is created. */
    JSAtom *finishAtom();

    template <size_t ArrayLength>
    bool append(const char (&array)[ArrayLength]) {
        return cb.append(array, array + ArrayLength - 1); /* No trailing '\0'. */
    }
};

inline
StringBuffer::StringBuffer(JSContext *cx)
  : cb(cx)
{}

inline bool
StringBuffer::reserve(size_t len)
{
    if (!checkLength(len))
        return false;
    return cb.reserve(len);
}

inline bool
StringBuffer::resize(size_t len)
{
    if (!checkLength(len))
        return false;
    return cb.resize(len);
}

inline bool
StringBuffer::append(const jschar c)
{
    if (!checkLength(cb.length() + 1))
        return false;
    return cb.append(c);
}

inline bool
StringBuffer::append(const jschar *chars, size_t len)
{
    if (!checkLength(cb.length() + len))
        return false;
    return cb.append(chars, len);
}

inline bool
StringBuffer::append(const jschar *begin, const jschar *end)
{
    if (!checkLength(cb.length() + (end - begin)))
        return false;
    return cb.append(begin, end);
}

inline bool
StringBuffer::append(JSString *str)
{
    JSLinearString *linear = str->ensureLinear(context());
    if (!linear)
        return false;
    size_t strLen = linear->length();
    if (!checkLength(cb.length() + strLen))
        return false;
    return cb.append(linear->chars(), strLen);
}

inline bool
StringBuffer::append(JSAtom *atom)
{
    size_t strLen = atom->length();
    if (!checkLength(cb.length() + strLen))
        return false;
    return cb.append(atom->chars(), strLen);
}

inline bool
StringBuffer::appendN(const jschar c, size_t n)
{
    if (!checkLength(cb.length() + n))
        return false;
    return cb.appendN(c, n);
}

inline bool
StringBuffer::appendInflated(const char *cstr, size_t cstrlen)
{
    size_t lengthBefore = length();
    if (!cb.growByUninitialized(cstrlen))
        return false;
#if DEBUG
    size_t oldcstrlen = cstrlen;
    bool ok = 
#endif
    js_InflateStringToBuffer(context(), cstr, cstrlen, begin() + lengthBefore, &cstrlen);
    JS_ASSERT(ok && oldcstrlen == cstrlen);
    return true;
}

inline jsint
StringBuffer::length() const
{
    JS_STATIC_ASSERT(jsint(JSString::MAX_LENGTH) == JSString::MAX_LENGTH);
    JS_ASSERT(cb.length() <= JSString::MAX_LENGTH);
    return jsint(cb.length());
}

inline bool
StringBuffer::checkLength(size_t length)
{
    return CheckStringLength(context(), length);
}

extern bool
ValueToStringBufferSlow(JSContext *cx, const Value &v, StringBuffer &sb);

inline bool
ValueToStringBuffer(JSContext *cx, const Value &v, StringBuffer &sb)
{
    if (v.isString())
        return sb.append(v.toString());

    return ValueToStringBufferSlow(cx, v, sb);
}

} /* namespace js */

JS_ALWAYS_INLINE void
JSRope::init(JSString *left, JSString *right, size_t length)
{
    d.lengthAndFlags = buildLengthAndFlags(length, ROPE_BIT);
    d.u1.left = left;
    d.s.u2.right = right;
}

JS_ALWAYS_INLINE JSRope *
JSRope::new_(JSContext *cx, JSString *left, JSString *right, size_t length)
{
    JSRope *str = (JSRope *)js_NewGCString(cx);
    if (!str)
        return NULL;
    str->init(left, right, length);
    return str;
}

JS_ALWAYS_INLINE void
JSDependentString::init(JSLinearString *base, const jschar *chars, size_t length)
{
    d.lengthAndFlags = buildLengthAndFlags(length, DEPENDENT_BIT);
    d.u1.chars = chars;
    d.s.u2.base = base;
}

JS_ALWAYS_INLINE JSDependentString *
JSDependentString::new_(JSContext *cx, JSLinearString *base, const jschar *chars, size_t length)
{
    /* Try to avoid long chains of dependent strings. */
    while (base->isDependent())
        base = base->asDependent().base();

    JS_ASSERT(base->isFlat());
    JS_ASSERT(chars >= base->chars() && chars < base->chars() + base->length());
    JS_ASSERT(length <= base->length() - (chars - base->chars()));

    JSDependentString *str = (JSDependentString *)js_NewGCString(cx);
    if (!str)
        return NULL;
    str->init(base, chars, length);
#ifdef DEBUG
    JSRuntime *rt = cx->runtime;
    JS_RUNTIME_METER(rt, liveDependentStrings);
    JS_RUNTIME_METER(rt, totalDependentStrings);
    JS_RUNTIME_METER(rt, liveStrings);
    JS_RUNTIME_METER(rt, totalStrings);
    JS_LOCK_RUNTIME_VOID(rt,
        (rt->strdepLengthSum += (double)length,
         rt->strdepLengthSquaredSum += (double)length * (double)length));
    JS_LOCK_RUNTIME_VOID(rt,
        (rt->lengthSum += (double)length,
         rt->lengthSquaredSum += (double)length * (double)length));
#endif
    return str;
}

JS_ALWAYS_INLINE void
JSFixedString::init(const jschar *chars, size_t length)
{
    d.lengthAndFlags = buildLengthAndFlags(length, FIXED_FLAGS);
    d.u1.chars = chars;
}

JS_ALWAYS_INLINE JSFixedString *
JSFixedString::new_(JSContext *cx, const jschar *chars, size_t length)
{
    JS_ASSERT(length <= MAX_LENGTH);
    JS_ASSERT(chars[length] == jschar(0));

    JSFixedString *str = (JSFixedString *)js_NewGCString(cx);
    if (!str)
        return NULL;
    str->init(chars, length);

    cx->runtime->stringMemoryUsed += length * 2;
#ifdef DEBUG
    JSRuntime *rt = cx->runtime;
    JS_RUNTIME_METER(rt, liveStrings);
    JS_RUNTIME_METER(rt, totalStrings);
    JS_LOCK_RUNTIME_VOID(rt,
        (rt->lengthSum += (double)length,
         rt->lengthSquaredSum += (double)length * (double)length));
#endif
    return str;
}

JS_ALWAYS_INLINE JSAtom *
JSFixedString::morphInternedStringIntoAtom()
{
    JS_ASSERT((d.lengthAndFlags & FLAGS_MASK) == JS_BIT(2));
    JS_STATIC_ASSERT(NON_STATIC_ATOM == JS_BIT(3));
    d.lengthAndFlags ^= (JS_BIT(2) | JS_BIT(3));
    return &asAtom();
}

JS_ALWAYS_INLINE JSInlineString *
JSInlineString::new_(JSContext *cx)
{
    return (JSInlineString *)js_NewGCString(cx);
}

JS_ALWAYS_INLINE jschar *
JSInlineString::init(size_t length)
{
    d.lengthAndFlags = buildLengthAndFlags(length, FIXED_FLAGS);
    d.u1.chars = d.inlineStorage;
    JS_ASSERT(lengthFits(length) || (isShort() && JSShortString::lengthFits(length)));
    return d.inlineStorage;
}

JS_ALWAYS_INLINE void
JSInlineString::resetLength(size_t length)
{
    d.lengthAndFlags = buildLengthAndFlags(length, FIXED_FLAGS);
    JS_ASSERT(lengthFits(length) || (isShort() && JSShortString::lengthFits(length)));
}

JS_ALWAYS_INLINE JSShortString *
JSShortString::new_(JSContext *cx)
{
    return js_NewGCShortString(cx);
}

JS_ALWAYS_INLINE void
JSShortString::initAtOffsetInBuffer(const jschar *chars, size_t length)
{
    JS_ASSERT(lengthFits(length + (chars - d.inlineStorage)));
    JS_ASSERT(chars >= d.inlineStorage && chars < d.inlineStorage + MAX_SHORT_LENGTH);
    d.lengthAndFlags = buildLengthAndFlags(length, FIXED_FLAGS);
    d.u1.chars = chars;
}

JS_ALWAYS_INLINE void
JSExternalString::init(const jschar *chars, size_t length, intN type, void *closure)
{
    d.lengthAndFlags = buildLengthAndFlags(length, FIXED_FLAGS);
    d.u1.chars = chars;
    d.s.u2.externalType = type;
    d.s.u3.externalClosure = closure;
}

JS_ALWAYS_INLINE JSExternalString *
JSExternalString::new_(JSContext *cx, const jschar *chars, size_t length, intN type, void *closure)
{
    JS_ASSERT(uintN(type) < JSExternalString::TYPE_LIMIT);
    JS_ASSERT(chars[length] == 0);

    JSExternalString *str = (JSExternalString *)js_NewGCExternalString(cx, type);
    if (!str)
        return NULL;
    str->init(chars, length, type, closure);
    cx->runtime->updateMallocCounter((length + 1) * sizeof(jschar));
    return str;
}

inline bool
JSAtom::fitsInSmallChar(jschar c)
{
    return c < SMALL_CHAR_LIMIT && toSmallChar[c] != INVALID_SMALL_CHAR;
}

inline bool
JSAtom::hasUnitStatic(jschar c)
{
    return c < UNIT_STATIC_LIMIT;
}

inline JSStaticAtom &
JSAtom::unitStatic(jschar c)
{
    JS_ASSERT(hasUnitStatic(c));
    return (JSStaticAtom &)unitStaticTable[c];
}

inline bool
JSAtom::hasIntStatic(int32 i)
{
    return uint32(i) < INT_STATIC_LIMIT;
}

inline JSStaticAtom &
JSAtom::intStatic(jsint i)
{
    JS_ASSERT(hasIntStatic(i));
    return (JSStaticAtom &)*intStaticTable[i];
}

inline JSLinearString *
JSAtom::getUnitStringForElement(JSContext *cx, JSString *str, size_t index)
{
    JS_ASSERT(index < str->length());
    const jschar *chars = str->getChars(cx);
    if (!chars)
        return NULL;
    jschar c = chars[index];
    if (c < UNIT_STATIC_LIMIT)
        return &unitStatic(c);
    return js_NewDependentString(cx, str, index, 1);
}

inline JSStaticAtom &
JSAtom::length2Static(jschar c1, jschar c2)
{
    JS_ASSERT(fitsInSmallChar(c1));
    JS_ASSERT(fitsInSmallChar(c2));
    size_t index = (((size_t)toSmallChar[c1]) << 6) + toSmallChar[c2];
    return (JSStaticAtom &)length2StaticTable[index];
}

inline JSStaticAtom &
JSAtom::length2Static(uint32 i)
{
    JS_ASSERT(i < 100);
    return length2Static('0' + i / 10, '0' + i % 10);
}

/* Get a static atomized string for chars if possible. */
inline JSStaticAtom *
JSAtom::lookupStatic(const jschar *chars, size_t length)
{
    switch (length) {
      case 1:
        if (chars[0] < UNIT_STATIC_LIMIT)
            return &unitStatic(chars[0]);
        return NULL;
      case 2:
        if (fitsInSmallChar(chars[0]) && fitsInSmallChar(chars[1]))
            return &length2Static(chars[0], chars[1]);
        return NULL;
      case 3:
        /*
         * Here we know that JSString::intStringTable covers only 256 (or at least
         * not 1000 or more) chars. We rely on order here to resolve the unit vs.
         * int string/length-2 string atom identity issue by giving priority to unit
         * strings for "0" through "9" and length-2 strings for "10" through "99".
         */
        JS_STATIC_ASSERT(INT_STATIC_LIMIT <= 999);
        if ('1' <= chars[0] && chars[0] <= '9' &&
            '0' <= chars[1] && chars[1] <= '9' &&
            '0' <= chars[2] && chars[2] <= '9') {
            jsint i = (chars[0] - '0') * 100 +
                      (chars[1] - '0') * 10 +
                      (chars[2] - '0');

            if (jsuint(i) < INT_STATIC_LIMIT)
                return &intStatic(i);
        }
        return NULL;
    }

    return NULL;
}

JS_ALWAYS_INLINE void
JSString::finalize(JSContext *cx)
{
    JS_ASSERT(!isStaticAtom() && !isShort());

    JS_RUNTIME_UNMETER(cx->runtime, liveStrings);

    if (isDependent())
        JS_RUNTIME_UNMETER(cx->runtime, liveDependentStrings);
    else if (isFlat())
        asFlat().finalize(cx->runtime);
    else
        JS_ASSERT(isRope());
}

inline void
JSFlatString::finalize(JSRuntime *rt)
{
    JS_ASSERT(!isShort());
    rt->stringMemoryUsed -= length() * 2;

    /*
     * This check depends on the fact that 'chars' is only initialized to the
     * beginning of inlineStorage. E.g., this is not the case for short strings.
     */
    if (chars() != d.inlineStorage)
        rt->free_(const_cast<jschar *>(chars()));
}

inline void
JSShortString::finalize(JSContext *cx)
{
    JS_ASSERT(isShort());
    JS_RUNTIME_UNMETER(cx->runtime, liveStrings);
}

inline void
JSAtom::finalize(JSRuntime *rt)
{
    JS_ASSERT(isAtom());
    if (arenaHeader()->getThingKind() == js::gc::FINALIZE_STRING)
        asFlat().finalize(rt);
    else
        JS_ASSERT(arenaHeader()->getThingKind() == js::gc::FINALIZE_SHORT_STRING);
}

inline void
JSExternalString::finalize(JSContext *cx)
{
    JS_RUNTIME_UNMETER(cx->runtime, liveStrings);
    if (JSStringFinalizeOp finalizer = str_finalizers[externalType()])
        finalizer(cx, this);
}

inline void
JSExternalString::finalize()
{
    JSStringFinalizeOp finalizer = str_finalizers[externalType()];
    if (finalizer) {
        /*
         * Assume that the finalizer for the permanently interned
         * string knows how to deal with null context.
         */
        finalizer(NULL, this);
    }
}

namespace js {

class RopeBuilder {
    JSContext *cx;
    JSString *res;

  public:
    RopeBuilder(JSContext *cx)
      : cx(cx), res(cx->runtime->emptyString)
    {}

    inline bool append(JSString *str) {
        res = js_ConcatStrings(cx, res, str);
        return !!res;
    }

    inline JSString *result() {
        return res;
    }
};

class StringSegmentRange
{
    /*
     * If malloc() shows up in any profiles from this vector, we can add a new
     * StackAllocPolicy which stashes a reusable freed-at-gc buffer in the cx.
     */
    Vector<JSString *, 32> stack;
    JSLinearString *cur;

    bool settle(JSString *str) {
        while (str->isRope()) {
            JSRope &rope = str->asRope();
            if (!stack.append(rope.rightChild()))
                return false;
            str = rope.leftChild();
        }
        cur = &str->asLinear();
        return true;
    }

  public:
    StringSegmentRange(JSContext *cx)
      : stack(cx), cur(NULL)
    {}

    JS_WARN_UNUSED_RESULT bool init(JSString *str) {
        JS_ASSERT(stack.empty());
        return settle(str);
    }

    bool empty() const {
        return cur == NULL;
    }

    JSLinearString *front() const {
        JS_ASSERT(!cur->isRope());
        return cur;
    }

    JS_WARN_UNUSED_RESULT bool popFront() {
        JS_ASSERT(!empty());
        if (stack.empty()) {
            cur = NULL;
            return true;
        }
        return settle(stack.popCopy());
    }
};

}  /* namespace js */

#endif /* jsstrinlines_h___ */
