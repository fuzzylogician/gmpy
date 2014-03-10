/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * gmpy2_truediv.c                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Python interface to the GMP or MPIR, MPFR, and MPC multiple precision   *
 * libraries.                                                              *
 *                                                                         *
 * Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,               *
 *           2008, 2009 Alex Martelli                                      *
 *                                                                         *
 * Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Case Van Horsen      *
 *                                                                         *
 * This file is part of GMPY2.                                             *
 *                                                                         *
 * GMPY2 is free software: you can redistribute it and/or modify it under  *
 * the terms of the GNU Lesser General Public License as published by the  *
 * Free Software Foundation, either version 3 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * GMPY2 is distributed in the hope that it will be useful, but WITHOUT    *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   *
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public    *
 * License for more details.                                               *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with GMPY2; if not, see <http://www.gnu.org/licenses/>    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This file implements the / operator, gmpy2.div() and context.div().
 *
 * Public API
 * ==========
 * The following function is available as part of GMPY2's C API. A NULL value
 * for context implies the function should use the currently active context.
 *
 *   GMPy_Number_TrueDiv(Number, Number, context|NULL)
 *
 * Private API
 * ===========
 *   GMPy_MPZ_TrueDiv_Slot
 *   GMPy_MPQ_TrueDiv_Slot
 *   GMPy_MPFR_TrueDiv_Slot
 *   GMPy_MPC_TrueDiv_Slot
 *
 *   GMPy_Integer_TrueDiv(Integer, Integer, context|NULL)
 *   GMPy_Rational_TrueDiv(Rational, Rational, context|NULL)
 *   GMPy_Real_TrueDiv(Real, Real, context|NULL)
 *   GMPy_Complex_TrueDiv(Complex, Complex, context|NULL)
 *
 *   GMPy_Context_TrueDiv(context, args)
 *
 */

/* Multiply two Integer objects (see gmpy2_convert.c). If an error occurs,
 * NULL is returned and an exception is set. If either x or y can't be
 * converted into an mpz, Py_NotImplemented is returned. */

/* Divide two Integer objects (see convert.c/isInteger) using true division.
 * If an error occurs, NULL is returned and an exception is set. If either x
 * or y can't be converted into an mpz, Py_NotImplemented is returned. */

static PyObject *
GMPy_Integer_TrueDiv(PyObject *x, PyObject *y, CTXT_Object *context)
{
    MPZ_Object *tempx, *tempy;
    mpq_t tempq;
    MPFR_Object *result;

    CHECK_CONTEXT_SET_EXPONENT(context);

    if (!(result = GMPy_MPFR_New(0, context)))
        return NULL;

    if (IS_INTEGER(x) && IS_INTEGER(y)) {
        tempx = GMPy_MPZ_From_Integer_Temp(x, context);
        tempy = GMPy_MPZ_From_Integer_Temp(y, context);
        if (!tempx || !tempy) {
            SYSTEM_ERROR("could not convert Integer to mpz");
            Py_XDECREF((PyObject*)tempx);
            Py_XDECREF((PyObject*)tempy);
            Py_DECREF((PyObject*)result);
            return NULL;
        }
        if (mpz_sgn(tempy->z) == 0) {
            ZERO_ERROR("division or modulo by zero");
            Py_XDECREF((PyObject*)tempx);
            Py_XDECREF((PyObject*)tempy);
            Py_DECREF((PyObject*)result);
            return NULL;
        }

        mpq_init(tempq);
        mpq_set_num(tempq, tempx->z);
        mpq_set_den(tempq, tempy->z);
        mpq_canonicalize(tempq);

        mpfr_clear_flags();

        result->rc = mpfr_set_q(result->f, tempq, GET_MPFR_ROUND(context));

        mpq_clear(tempq);
        Py_DECREF((PyObject*)tempx);
        Py_DECREF((PyObject*)tempy);
        MPFR_CLEANUP_2(result, context, "division");
        return (PyObject*)result;
    }

    Py_DECREF((PyObject*)result);
    Py_RETURN_NOTIMPLEMENTED;
}

/* Implement true division for MPZ_Object. On entry, one of the two arguments
 * must be an MPZ_Object. If the other object is an Integer, return an
 * MPZ_Object. If the other object isn't an MPZ_Object, call the appropriate
 * function. If no appropriate function can be found, return NotImplemented.
 */

static PyObject *
GMPy_MPZ_TrueDiv_Slot(PyObject *x, PyObject *y)
{
    if (IS_INTEGER(x) && IS_INTEGER(y))
        return GMPy_Integer_TrueDiv(x, y, NULL);

    if (IS_RATIONAL(x) && IS_RATIONAL(y))
        return GMPy_Rational_TrueDiv(x, y, NULL);

    if (IS_REAL(x) && IS_REAL(y))
        return GMPy_Real_TrueDiv(x, y, NULL);

    if (IS_COMPLEX(x) && IS_COMPLEX(y))
        return GMPy_Complex_TrueDiv(x, y, NULL);

    Py_RETURN_NOTIMPLEMENTED;
}

#ifdef PY2
static PyObject *
GMPy_MPZ_Div2_Slot(PyObject *x, PyObject *y)
{
    if (IS_INTEGER(x) && IS_INTEGER(y))
        return GMPy_Integer_FloorDiv(x, y, NULL);

    if (IS_RATIONAL(x) && IS_RATIONAL(y))
        return GMPy_Rational_TrueDiv(x, y, NULL);

    if (IS_REAL(x) && IS_REAL(y))
        return GMPy_Real_TrueDiv(x, y, NULL);

    if (IS_COMPLEX(x) && IS_COMPLEX(y))
        return GMPy_Complex_TrueDiv(x, y, NULL);

    Py_RETURN_NOTIMPLEMENTED;
}
#endif

static PyObject *
GMPy_Rational_TrueDiv(PyObject *x, PyObject *y, CTXT_Object *context)
{
    MPQ_Object *result, *tempx, *tempy;

    CHECK_CONTEXT_SET_EXPONENT(context);

    if (!(result = GMPy_MPQ_New(context)))
        return NULL;

    if (IS_RATIONAL(x) && IS_RATIONAL(y)) {
        tempx = GMPy_MPQ_From_Number_Temp(x, context);
        tempy = GMPy_MPQ_From_Number_Temp(y, context);
        if (!tempx || !tempy) {
            SYSTEM_ERROR("could not convert Rational to mpq");
            goto error;
        }
        if (mpq_sgn(tempy->q) == 0) {
            ZERO_ERROR("division or modulo by zero");
            goto error;
        }

        mpq_div(result->q, tempx->q, tempy->q);
        Py_DECREF((PyObject*)tempx);
        Py_DECREF((PyObject*)tempy);
        return (PyObject*)result;
    }

    Py_DECREF((PyObject*)result);
    Py_RETURN_NOTIMPLEMENTED;

  error:
    Py_XDECREF((PyObject*)tempx);
    Py_XDECREF((PyObject*)tempy);
    Py_DECREF((PyObject*)result);
    return NULL;
}

static PyObject *
GMPy_MPQ_TrueDiv_Slot(PyObject *x, PyObject *y)
{
    if (IS_RATIONAL(x) && IS_RATIONAL(y))
        return GMPy_Rational_TrueDiv(x, y, NULL);

    if (IS_REAL(x) && IS_REAL(y))
        return GMPy_Real_TrueDiv(x, y, NULL);

    if (IS_COMPLEX(x) && IS_COMPLEX(y))
        return GMPy_Complex_TrueDiv(x, y, NULL);

    Py_RETURN_NOTIMPLEMENTED;
}

/* Attempt true division of two numbers and return an mpfr. The code path is
 * optimized by checking for mpfr objects first. Returns Py_NotImplemented if
 * both objects are not valid reals.  */

static PyObject *
GMPy_Real_TrueDiv(PyObject *x, PyObject *y, CTXT_Object *context)
{
    MPFR_Object *result;

    CHECK_CONTEXT_SET_EXPONENT(context);

    if (!(result = GMPy_MPFR_New(0, context)))
        return NULL;

    /* This only processes mpfr if the exponent is still in-bounds. Need
     * to handle the rare case at the end. */

    if (MPFR_CheckAndExp(x) && MPFR_CheckAndExp(y)) {
        mpfr_clear_flags();
        result->rc = mpfr_div(result->f, MPFR(x), MPFR(y),
                              GET_MPFR_ROUND(context));
        goto done;
    }

    if (MPFR_CheckAndExp(x)) {
        if (PyIntOrLong_Check(y)) {
            mpz_t tempz;
            mpir_si temp_si;
            int overflow;

            temp_si = PyLong_AsSIAndOverflow(y, &overflow);
            if (overflow) {
                mpz_inoc(tempz);
                mpz_set_PyIntOrLong(tempz, y);
                mpfr_clear_flags();
                result->rc = mpfr_div_z(result->f, MPFR(x), tempz,
                                        GET_MPFR_ROUND(context));
                mpz_cloc(tempz);
                goto done;
            }
            else {
                mpfr_clear_flags();
                result->rc = mpfr_div_si(result->f, MPFR(x), temp_si,
                                         GET_MPFR_ROUND(context));
                goto done;
            }
        }

        if (CHECK_MPZANY(y)) {
            mpfr_clear_flags();
            result->rc = mpfr_div_z(result->f, MPFR(x), MPZ(y),
                                    GET_MPFR_ROUND(context));
            goto done;
        }

        if (IS_RATIONAL(y)) {
            MPQ_Object *tempy;

            if (!(tempy = GMPy_MPQ_From_Number_Temp(y, context))) {
                Py_DECREF((PyObject*)result);
                return NULL;
            }
            mpfr_clear_flags();
            result->rc = mpfr_div_q(result->f, MPFR(x), tempy->q,
                                    GET_MPFR_ROUND(context));
            Py_DECREF((PyObject*)tempy);
            goto done;
        }

        if (PyFloat_Check(y)) {
            mpfr_clear_flags();
            result->rc = mpfr_div_d(result->f, MPFR(x), PyFloat_AS_DOUBLE(y),
                                    GET_MPFR_ROUND(context));
            goto done;
        }
    }

    if (MPFR_CheckAndExp(y)) {
        if (PyIntOrLong_Check(x)) {
            mpir_si temp_si;
            int overflow;

            temp_si = PyLong_AsSIAndOverflow(x, &overflow);
            if (!overflow) {
                mpfr_clear_flags();
                result->rc = mpfr_si_div(result->f, temp_si, MPFR(y),
                                         GET_MPFR_ROUND(context));
                goto done;
            }
        }

        /* Since mpfr_z_div does not exist, this combination is handled at the
         * end by converting x to an mpfr. Ditto for rational.*/

        if (PyFloat_Check(x)) {
            mpfr_clear_flags();
            result->rc = mpfr_d_div(result->f, PyFloat_AS_DOUBLE(x),
                                    MPFR(y), GET_MPFR_ROUND(context));
            goto done;
        }
    }

    /* In addition to handling PyFloat + PyFloat, the rare case when the
     * exponent bounds have been changed is handled here. See
     * Pympfr_From_Real() for details. */

    if (IS_REAL(x) && IS_REAL(y)) {
        MPFR_Object *tempx, *tempy;

        tempx = GMPy_MPFR_From_Real(x, 1, context);
        tempy = GMPy_MPFR_From_Real(y, 1, context);
        if (!tempx || !tempy) {
            Py_XDECREF((PyObject*)tempx);
            Py_XDECREF((PyObject*)tempy);
            Py_DECREF((PyObject*)result);
            return NULL;
        }
        mpfr_clear_flags();
        result->rc = mpfr_div(result->f, tempx->f, tempy->f,
                              GET_MPFR_ROUND(context));
        Py_DECREF((PyObject*)tempx);
        Py_DECREF((PyObject*)tempy);
        goto done;
    }

    Py_DECREF((PyObject*)result);
    Py_RETURN_NOTIMPLEMENTED;

  done:
    MPFR_CLEANUP_2(result, context, "division");
    return (PyObject*)result;
}

static PyObject *
GMPy_MPFR_TrueDiv_Slot(PyObject *x, PyObject *y)
{
    if (IS_REAL(x) && IS_REAL(y))
        return GMPy_Real_TrueDiv(x, y, NULL);

    if (IS_COMPLEX(x) && IS_COMPLEX(y))
        return GMPy_Complex_TrueDiv(x, y, NULL);

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *
GMPy_Complex_TrueDiv(PyObject *x, PyObject *y, CTXT_Object *context)
{
    MPC_Object *result = NULL;

    CHECK_CONTEXT_SET_EXPONENT(context);

    if (!(result = GMPy_MPC_New(0, 0, context)))
        return NULL;

    if (MPC_CheckAndExp(x) && MPC_CheckAndExp(y)) {
        if (MPC_IS_ZERO_P(y)) {
            context->ctx.divzero = 1;
            if (context->ctx.traps & TRAP_DIVZERO) {
                GMPY_DIVZERO("'mpc' division by zero");
                Py_DECREF((PyObject*)result);
                return NULL;
            }
        }
        result->rc = mpc_div(result->c, MPC(x), MPC(y),
                             GET_MPC_ROUND(context));
        goto done;
    }

    if (IS_COMPLEX(x) && IS_COMPLEX(y)) {
        MPC_Object *tempx, *tempy;

        tempx = GMPy_MPC_From_Complex(x, 1, 1, context);
        tempy = GMPy_MPC_From_Complex(y, 1, 1, context);
        if (!tempx || !tempy) {
            Py_XDECREF((PyObject*)tempx);
            Py_XDECREF((PyObject*)tempy);
            Py_DECREF((PyObject*)result);
            return NULL;
        }
        result->rc = mpc_div(result->c, tempx->c, tempy->c,
                             GET_MPC_ROUND(context));
        Py_DECREF((PyObject*)tempx);
        Py_DECREF((PyObject*)tempy);
        goto done;
    }

    Py_DECREF((PyObject*)result);
    Py_RETURN_NOTIMPLEMENTED;

  done:
    MPC_CLEANUP_2(result, context, "division");
    return (PyObject*)result;
}

static PyObject *
GMPy_MPC_TrueDiv_Slot(PyObject *x, PyObject *y)
{
    return GMPy_Complex_TrueDiv(x, y, NULL);
}

PyDoc_STRVAR(GMPy_doc_truediv,
"div(x, y) -> number\n\n"
"Return x / y; uses true division.");

static PyObject *
GMPy_Number_TrueDiv(PyObject *x, PyObject *y, CTXT_Object *context)
{
    if (IS_INTEGER(x) && IS_INTEGER(y))
        return GMPy_Integer_TrueDiv(x, y, context);

    if (IS_RATIONAL(x) && IS_RATIONAL(y))
        return GMPy_Rational_TrueDiv(x, y, context);

    if (IS_REAL(x) && IS_REAL(y))
        return GMPy_Real_TrueDiv(x, y, context);

    if (IS_COMPLEX(x) && IS_COMPLEX(y))
        return GMPy_Complex_TrueDiv(x, y, context);

    TYPE_ERROR("div() argument type not supported");
    return NULL;
}

PyDoc_STRVAR(GMPy_doc_context_truediv,
"context.div(x, y) -> number\n\n"
"Return x / y; uses true division.");

static PyObject *
GMPy_Context_TrueDiv(PyObject *self, PyObject *args)
{
    PyObject *result;
    CTXT_Object *context = NULL;

    if (PyTuple_GET_SIZE(args) != 2) {
        TYPE_ERROR("div() requires 2 arguments.");
        return NULL;
    }

    if (self && CTXT_Check(self)) {
        /* If we are passed a read-only context, make a copy of it before
         * proceeding. Remember to decref context when we're done. */

        if (((CTXT_Object*)self)->ctx.readonly) {
            context = (CTXT_Object*)GMPy_CTXT_Copy(self, NULL);
            if (!context)
                return NULL;
        }
        else {
            context = (CTXT_Object*)self;
            Py_INCREF((PyObject*)context);
        }
    }
    else {
        CHECK_CONTEXT_SET_EXPONENT(context);
        Py_INCREF((PyObject*)context);
    }

    result = GMPy_Number_TrueDiv(PyTuple_GET_ITEM(args, 0),
                                 PyTuple_GET_ITEM(args, 1),
                                 context);
    Py_DECREF((PyObject*)context);
    return result;
}
