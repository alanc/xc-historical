;;; -*- Mode: LISP; Syntax: Common-lisp; Package: USER; Base: 10; Lowercase: Yes -*-

;; Note: File mode-lines don't have:
;;    PACKAGE: (XLIB :USE (CL))
;;  because the TI Explorer doesn't fully support it.
;;  Define the XLIB package here instead.

(in-package 'xlib :use '(lisp))

;;;
;;;			 TEXAS INSTRUMENTS INCORPORATED
;;;				  P.O. BOX 2909
;;;			       AUSTIN, TEXAS 78769
;;;
;;; Copyright (C) 1987 Texas Instruments Incorporated.
;;;
;;; Permission is granted to any individual or institution to use, copy, modify,
;;; and distribute this software, provided that this complete copyright and
;;; permission notice is maintained, intact, in all copies and supporting
;;; documentation.
;;;
;;; Texas Instruments Incorporated provides this software "as is" without
;;; express or implied warranty.
;;;

#+(and lispm (not genera))
(global:defsystem CLX
  (:pathname-default "clx:clx;")
  (:patchable "clx:patch;" clx-ti)
  (:initial-status :experimental)

  (:module depdefs "depdefs")
  (:module clx "clx")
  (:module dependent "dependent")
  (:module macros "macros")
  (:module bufmac "bufmac")
  (:module buffer "buffer")
  (:module display "display")
  (:module gcontext "gcontext")
  (:module requests "requests")
  (:module input "input")
  (:module fonts "fonts")
  (:module graphics "graphics")
  (:module text "text")
  (:module attributes "attributes")
  (:module translate "translate")
  (:module keysyms "keysyms")
  (:module manager "manager")
  (:module image "image")
  (:module resource "resource")
  (:module doc "doc")

  (:compile-load depdefs)
  (:compile-load clx
   (:fasload depdefs))
  (:compile-load dependent
   (:fasload depdefs clx))
  ;; Macros only needed for compilation
  (:skip :compile-load macros
   (:fasload depdefs clx dependent))
  ;; Bufmac only needed for compilation
  (:skip :compile-load bufmac
   (:fasload depdefs clx dependent macros))
  (:compile-load buffer
   (:fasload depdefs clx dependent macros bufmac))
  (:compile-load display
   (:fasload depdefs clx dependent macros bufmac buffer))
  (:compile-load gcontext
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load requests
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load input
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load fonts
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load graphics
   (:fasload depdefs clx dependent macros fonts bufmac buffer display fonts))
  (:compile-load text
   (:fasload depdefs clx dependent macros fonts bufmac buffer display gcontext fonts))
  (:compile-load-init attributes
   (dependent)					;<- There may be other modules needed here.
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load translate
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load keysyms
   (:fasload depdefs clx dependent macros bufmac buffer display translate))
  (:compile-load manager
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load image
   (:fasload depdefs clx dependent macros bufmac buffer display))
  (:compile-load resource)
  (:auxiliary doc)
  )


;;; Symbolics Genera has two different schedulers, one in 7.2 and the other in 7.3 and 7.4.
;;; Define a feature so we can use different code for them.
#+Genera
(eval-when (compile load eval)
  (multiple-value-bind (ignore status)
      (find-symbol "WITH-LOCK" (find-package 'process))
    (when (eq status :external)
      (pushnew :clx-genera-new-scheduler *features*))))

#+Genera
(scl:defsystem CLX
    (:default-pathname "SYS:X11;CLX;"
     :default-package "XLIB"
     :pretty-name "CLX"
     :maintaining-sites (:scrc)
     :distribute-sources t
     :distribute-binaries t
     :source-category :basic)
  (:module doc ("doc")
	   (:type :lisp-example))
  (:module depdefs ("depdefs" #+CLX-Genera-New-Scheduler "generalock"))
  (:module clx ("clx")
	   (:uses-definitions-from depdefs))
  (:module dependent ("dependent")
	   (:uses-definitions-from clx))
  (:module macros ("macros")
	   (:uses-definitions-from dependent))
  (:module bufmac ("bufmac")
	   (:uses-definitions-from dependent macros))
  (:module buffer ("buffer")
	   (:uses-definitions-from dependent macros bufmac))
  (:module display ("display")
	   (:uses-definitions-from dependent macros bufmac buffer))
  (:module gcontext ("gcontext")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module requests ("requests")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module input ("input")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module fonts ("fonts")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module graphics ("graphics")
	   (:uses-definitions-from dependent macros bufmac fonts))
  (:module text ("text")
	   (:uses-definitions-from dependent macros bufmac gcontext fonts))
  (:module attributes ("attributes")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module translate ("translate")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module keysyms ("keysyms")
	   (:uses-definitions-from translate))
  (:module manager ("manager")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module image ("image")
	   (:uses-definitions-from dependent macros bufmac display))
  (:module resource ("resource"))
  )

#+lucid
(defvar *foreign-libraries* '("-lc")) ; '("-lresolv" "-lc") for some sites

#+lucid
(defun clx-foreign-files ()
  #-(or lcl3.0 vax) (load "make-sequence-patch")
  #+(and apollo (not lcl3.0))
  (lucid::define-foreign-function '(xlib::connect-to-server "connect_to_server")
      '((:val host    :string)
	(:val display :integer32))
    :integer32)
  #+(and (not apollo) (not lcl3.0))
  (lucid::define-c-function xlib::connect-to-server
      (host display)
    :result-type :integer)
  #+lcl3.0
  (lucid::def-foreign-function (xlib::connect-to-server 
				 (:language :c)
				 (:return-type :signed-32bit))
      (host :simple-string) (display :signed-32bit))
  (unintern 'display)
  #+apollo
  (lucid::load-foreign-file "socket" :preserve-pathname t)
  #-apollo
  (lucid::load-foreign-files '("socket.o") *foreign-libraries*))

#-lispm
(defun compile-clx (&optional pathname-defaults)
  (#+lcl3.0 lucid::with-deferred-warnings
   #-lcl3.0 progn
   (let ((*default-pathname-defaults*
	   (or pathname-defaults *default-pathname-defaults*)))
     (declare (special *default-pathname-defaults*))
     #+lucid
     (clx-foreign-files)
     #+kcl
     (load "tcpinit")
     #+excl
     (progn
       (compile-file "excldep")
       (load "excldep"))
     (compile-file "depdefs")
     (load "depdefs")
     (compile-file "clx")
     (load "clx")
     (compile-file "dependent")
     (load "dependent")
     (compile-file "macros")
     (load "macros")
     (compile-file "bufmac")
     (load "bufmac")
     (compile-file "buffer")
     (load "buffer")
     (compile-file "display")
     (load "display")
     (compile-file "gcontext")
     (load "gcontext")
     (compile-file "requests")
     (load "requests")
     (compile-file "input")
     (load "input")
     (compile-file "fonts")
     (load "fonts")
     (compile-file "graphics")
     (load "graphics")
     (compile-file "text")
     (load "text")
     (compile-file "attributes")
     (load "attributes")
     (compile-file "translate")
     (load "translate")
     (compile-file "keysyms")
     (load "keysyms")
     (compile-file "manager")
     (load "manager")
     (compile-file "image")
     (load "image")
     (compile-file "resource")
     (load "resource")
     )))

#-lispm
(defun load-clx (&optional pathname-defaults macros-p)
  (let ((*default-pathname-defaults*
	  (or pathname-defaults *default-pathname-defaults*)))
    (declare (special *default-pathname-defaults*))
    #+lucid
    (clx-foreign-files)
    #+kcl
    (load "tcpinit")
    #+excl
    (load "excldep")
    (load "depdefs")
    (load "clx")
    (load "dependent")
    (when macros-p
      (load "macros")
      (load "bufmac"))
    (load "buffer")
    (load "display")
    (load "gcontext")
    (load "requests")
    (load "input")
    (load "fonts")
    (load "graphics")
    (load "text")
    (load "attributes")
    (load "translate")
    (load "keysyms")
    (load "manager")
    (load "image")
    (load "resource")
    ))
