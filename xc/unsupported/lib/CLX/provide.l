;;; -*- Mode: LISP; Syntax: Common-lisp; Base: 10; Lowercase: Yes; Package: USER;  -*-

;;;; Module definition for CLX

;;; This file is a Common Lisp Module description, but you will have to edit
;;; it to meet the needs of your site.

;;; Ideally, this file (or a file that loads this file) should be
;;; located in the system directory that REQUIRE searches.  Thus a user
;;; would say
;;;			(require 'clx)
;;; to load CLX.  If there is no such registry, then the user must
;;; put in a site specific
;;;			(require 'clx <pathname-of-this-file>)
;;;

(in-package 'user)

(provide 'clx)

;;; Load the defsystem file from the source directory.  You may
;;; want to include an explicit extension (such as ".l" or ".lisp").
;;;
(load "/src/local/clx/defsystem.l")

;;; The binary files for a particular lisp implementation and architecture.
;;;
(xlib:load-clx
  #+(and lucid sun      sparc ) "/src/local/clx/lucid.sparc/"
  #+(and lucid sun (not sparc)) "/src/local/clx/lucid.sun3/"
  #+(and lucid hp)              "/src/local/clx/lucid.hp9000/"
  #+(and kcl            sparc ) "/src/local/clx/akcl.sparc/"
  #+(and kcl       (not sparc)) "/src/local/clx/akcl.sun3/")
