;;; -*- Mode: common-lisp; Package: USER; Base: 10; Lowercase: Yes -*-
;;;
;;; CLX -- excldep.cl
;;;
;;; Copyright (c) 1987, 1988, 1989 Franz Inc, Berkeley, Ca.
;;;
;;; Permission is granted to any individual or institution to use, copy,
;;; modify, and distribute this software, provided that this complete
;;; copyright and permission notice is maintained, intact, in all copies and
;;; supporting documentation.
;;;
;;; Franz Incorporated provides this software "as is" without
;;; express or implied warranty.
;;;

(in-package :xlib :use '(:foreign-functions :lisp :excl))

(eval-when (load)
  (provide :clxexcldep)
  (provide :clx))

(require :foreign)
(require :process)			; Needed even if scheduler is not
					; running.  (Must be able to make
					; a process-lock.)

(import '(excl::if* excl::defcmacro))

;;
;; The following is a suggestion.  If you comment out this form be
;; prepared for possible deadlock, since no interrupts will be recognized
;; while reading from the X socket if the scheduler is not running.
;;
;; Note -- this really should be in defsystem.cl, since it only matters when
;; compiling, but it's here and loaded into CLX.fasl for safety.
;;
(setq compiler::generate-interrupt-checks-switch
  (compile nil '(lambda (safety size speed)
		   (declare (ignore size))
		  (or (< speed 3) (> safety 0)))))


(eval-when (eval compile load)
  (let ((x '#(1)))
    (if (not (eq 0 (sys::memref x
				#.(comp::mdparam 'comp::md-svector-data0-adj)
				0 :unsigned-byte)))
	(pushnew :clx-little-endian *features*)))
  #+allegro
  (if (not (boundp 'excl::cl-major-version-number))
      (pushnew :allegro-v2.2 *features*))
  
  (proclaim '(special mp::*scheduler-stack-group* mp::*current-process*))
  (if (not (boundp 'mp::*scheduler-stack-group*))
      (setq mp::*scheduler-stack-group* nil)))


(defmacro define-condition (name (parent-type) &optional slots &rest args)
  `(excl::define-condition ,name (,parent-type) ,slots ,@args))


(defconstant type-pred-alist
  '(
    (card8  . card8p)
    (card16 . card16p)
    (card29 . card29p)
    (card32 . card32p)
    (int8   . int8p)
    (int16  . int16p)
    (int32  . int32p)
    (mask16 . card16p)
    (mask32 . card32p)
    (pixel  . card32p)
    (resource-id . card29p)
    (keysym . card32p)
    ))

;; This (if (and ...) t nil) stuff has a purpose -- it lets the sun4 compiler
;; opencode the `and'.

(defun card8p (x)
  (declare (optimize (speed 3) (safety 0))
	   (fixnum x))
  (if (and (fixnump x) (> #.(expt 2 8) x) (>= x 0))
      t
    nil))

(defcmacro card8p (x)
  (let ((xx (gensym)))
    `(let ((,xx ,x))
       (declare (optimize (speed 3) (safety 0))
		(fixnum ,xx))
       (and (fixnump ,xx) (> #.(expt 2 8) ,xx) (>= ,xx 0)))))

(defun card16p (x)
  (declare (optimize (speed 3) (safety 0))
	   (fixnum x))
  (if (and (fixnump x) (> #.(expt 2 16) x) (>= x 0))
      t
    nil))

(defcmacro card16p (x)
  (let ((xx (gensym)))
    `(let ((,xx ,x))
       (declare (optimize (speed 3) (safety 0))
		(fixnum ,xx))
       (and (fixnump ,xx) (> #.(expt 2 16) ,xx) (>= ,xx 0)))))

;; These next two are too large to macroexpand.

(defun card29p (x)
  (declare (optimize (speed 3) (safety 0)))
  (if (or (and (fixnump x) (>= (the fixnum x) 0))
	  (and (bignump x) (> #.(expt 2 29) (the bignum x))
	       (>= (the bignum x) 0)))
      t
    nil))

(defun card32p (x)
  (declare (optimize (speed 3) (safety 0)))
  (if (or (and (fixnump x) (>= (the fixnum x) 0))
	  (and (bignump x) (> #.(expt 2 32) (the bignum x))
	       (>= (the bignum x) 0)))
      t
    nil))

(defun int8p (x)
  (declare (optimize (speed 3) (safety 0))
	   (fixnum x))
  (if (and (fixnump x) (> #.(expt 2 7) x) (>= x #.(expt -2 7)))
      t
    nil))

(defcmacro int8p (x)
  (let ((xx (gensym)))
    `(let ((,xx ,x))
       (declare (optimize (speed 3) (safety 0))
		(fixnum ,xx))
       (and (fixnump ,xx) (> #.(expt 2 7) ,xx) (>= ,xx #.(expt -2 7))))))

(defun int16p (x)
  (declare (optimize (speed 3) (safety 0))
	   (fixnum x))
  (if (and (fixnump x) (> #.(expt 2 15) x) (>= x #.(expt -2 15)))
      t
    nil))

(defcmacro int16p (x)
  (let ((xx (gensym)))
    `(let ((,xx ,x))
       (declare (optimize (speed 3) (safety 0))
		(fixnum ,xx))
       (and (fixnump ,xx) (> #.(expt 2 15) ,xx) (>= ,xx #.(expt -2 15))))))

(defun int32p (x)
  (declare (optimize (speed 3) (safety 0)))
  (if (or (fixnump x)
	  (and (bignump x) (> #.(expt 2 31) (the bignum x))
	       (>= (the bignum x) #.(expt -2 31))))
      t
    nil))

(comp::def-tr comp::new-tr-typep typep (form type)
   (let (ent)
      (if* (and (consp type)
		(eq 'quote (car type))
		(consp (cdr type)))
	 then (setq ent (franz:assq (cadr type) type-pred-alist)))
      (if* ent
	 then `(,(cdr ent) ,form)
	 else (if* (and (consp type)
			(eq 'quote (car type))
			(consp (cdr type)))
		 then (setq ent (franz:assq (cadr type)
					    excl::type-pred-alist)))
	      (if* ent
		 then `(,(cdr ent) ,form)
		 else (comp::no-transform)))))


(defun fd-char-avail-p (fd)
   ;; return t if there is a character available for reading or on error
   (multiple-value-bind (available-p errcode)
	 (comp::.primcall-sargs 'sys::filesys excl::fs-char-avail fd)
      (if* errcode
	 then t
	 else available-p)))


(when
    (plusp
     (ff:get-entry-points
      (make-array 1 :initial-contents
		  (list (ff:convert-to-lang "c_read_bytes" :language :c)))
      (make-array 1 :element-type '(unsigned-byte 32))))
  (ff:remove-entry-point (ff:convert-to-lang "c_read_bytes" :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "c_read_bytes_interruptible"
					     :language :c))
  (load "excldep.o"))


;;
;; Suns need a special hack so that you can build CLX on a OS3.* machine and
;; use it on a OS4.0 machine.  This is because the hostent structure changed
;; incompatibly.
;;
#+(or :sun3 :sun4)
(when
    (plusp (ff:get-entry-points
	    (make-array 1 :initial-contents
			(list (ff:convert-to-lang "connect_to_server_3_x"
						  :language :c)))
	    (make-array 1 :element-type '(unsigned-byte 32))))

  (ff:remove-entry-point (ff:convert-to-lang "connect_to_server_3_x"
					     :language :c))
  (ff:remove-entry-point (ff:convert-to-lang "connect_to_server_4_x"
					     :language :c))
  (load "socket3x.o")
  (load "socket4x.o")) 

#-(or :sun3 :sun4)
(when
    (plusp (ff:get-entry-points
	    (make-array 1 :initial-contents
			(list (ff:convert-to-lang "connect_to_server"
						  :language :c)))
	    (make-array 1 :element-type '(unsigned-byte 32))))
  
  (ff:remove-entry-point (ff:convert-to-lang "connect_to_server" :language :c))
  (load "socket.o"))

(ff:defforeign-list `((xlib::connect-to-server
		       :entry-point
		       #+(or :sun3 :sun4)
		       ,(if (probe-file "/lib/ld.so")
			    (ff:convert-to-lang "connect_to_server_4_x"
						:language :c)
			  (ff:convert-to-lang "connect_to_server_3_x"
						:language :c))
		       #-(or :sun3 :sun4)
		       ,(ff:convert-to-lang "connect_to_server" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :arguments (string fixnum))
		      (xlib::c-read-bytes
		       :entry-point
		       ,(ff:convert-to-lang "c_read_bytes" :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :call-direct t
		       :callback nil
		       :allow-other-keys t
		       :arguments (fixnum (simple-array (unsigned-byte 8) (*))
				   fixnum fixnum))
		      (xlib::c-read-bytes-interruptible
		       :entry-point
		       ,(ff:convert-to-lang "c_read_bytes_interruptible"
			 :language :c)
		       :return-type :fixnum
		       :arg-checking nil
		       :call-direct t
		       :callback nil
		       :allow-other-keys t
		       :arguments (fixnum (simple-array (unsigned-byte 8) (*))
				   fixnum fixnum))))


(in-package :excl)

#-allegro
(defun type-array-element-type-to-array (type &aux temp) 
   ;; type is a type descriptor, return a descriptor which tells
   ;; the array code what kind of array to make

   ; convert the given element type to one of the symbols which
   ; is in the car of the array-descriptors list
   ;(msg "beginning type is " type 'N)
   (if* (symbolp type)
      then (if* (franz:memq type '(t bit string-char fixnum))
	      thenret	; it is ok as it is
	      else (let ((temp (get type 'deftype-expander)))
		     (if* temp
			then
			     (return-from type-array-element-type-to-array
			       (type-array-element-type-to-array
				(funcall temp (list type))))
			else
			     (setq type (case type
					  (standard-char 'string-char)
					  ((single-float short-float) 'single-float)
					  ((double-float long-float) 'double-float)
					  (t t))))))
    elseif (consp type)
      then (setq type
		 (case (car type)
		    (mod (if* (integerp (setq temp (cadr type)))
			    then (cond ((< temp 1) t)
				       ((<= temp 2) 'bit)
				       ((<= temp 256) 'ubyte)
				       ((<= temp 65536) 'uword)
				       ((<= temp 4294967296) 'ulong)
				       (t t))
			    else t))
		    (signed-byte
		       (if* (integerp (setq temp (cadr type)))
			  then (cond ((<= temp 0) t)
				     ((<= temp 8) 'byte)
				     ((<= temp 16) 'word)
				     ((<= temp 29) 'fixnum)
				     ((<= temp 32) 'long)
				     (t  t))
			  else t))
		    (unsigned-byte
		       (if* (integerp (setq temp (cadr type)))
			  then (cond ((<= temp 0) t)
				     ((<= temp 8) 'ubyte)
				     ((<= temp 16) 'uword)
				     ((<= temp 32) 'ulong)
				     (t  t))
			  else t))
		    (t t)))
      else (setq type t))
   ; type is now one of the valid types.  We return a descriptor
   ; based on that name
   ;(msg "resulting type is " type 'N)
   (let ((res (franz:assq type array-descriptors)))
      ;(msg " resulting decriptor " res 'N)
      res))


#+(or (not allegro) allegro-v2.2)
(defun make-sequence (type length &rest rest &key initial-element)
  "Returns a sequence of the given Type and Length, with elements initialized
  to :Initial-Element."
  (declare (fixnum length)
	   (ignore initial-element))
  (case (type-specifier type)
    (list (apply #'make-list length rest))
    ((simple-string string)
     (apply #'make-string length rest))
    ((array simple-array vector simple-vector)
     (if* (listp type)
	 then (apply #'make-array length :element-type (cadr type) rest)
	 else (apply #'make-array length rest)))
    ((bit-vector simple-bit-vector)
     (apply #'make-array length :element-type 'bit rest))
    (t
     ;; Now, we can either have a user-defined type symbol, or an error.
     (if* (symbolp type)
	then (let ((temp (get type 'excl::deftype-expander)))
	       (if* temp
		  then (cond (rest (return-from make-sequence
				     (make-sequence (funcall temp (list type)) length
						   :initial-element (cadr rest))))
			      (t (return-from make-sequence (make-sequence
				   (funcall temp (list type)) length)))))))
     (error "~s is a bad type specifier for sequences." type ))))
