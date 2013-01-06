;;; LFort Code-Completion minor mode, for use with C/Objective-C/C++.

;;; Commentary:

;; This minor mode uses LFort's command line interface for code
;; completion to provide code completion results for C, Objective-C,
;; and C++ source files. When enabled, LFort will provide
;; code-completion results in a secondary buffer based on the code
;; being typed. For example, after typing "struct " (triggered via the
;; space), LFort will provide the names of all structs visible from
;; the current scope. After typing "p->" (triggered via the ">"),
;; LFort will provide the names of all of the members of whatever
;; class/struct/union "p" points to. Note that this minor mode isn't
;; meant for serious use: it is meant to help experiment with code
;; completion based on LFort. It needs your help to make it better!
;;
;; To use the LFort code completion mode, first make sure that the
;; "lfort" variable below refers to the "lfort" executable,
;; which is typically installed in libexec/. Then, place
;; lfort-completion-mode.el somewhere in your Emacs load path. You can
;; add a new load path to Emacs by adding some like the following to
;; your .emacs:
;;
;;   (setq load-path (cons "~/.emacs.d" load-path))
;;
;; Then, use
;;
;;   M-x load-library
;;
;; to load the library in your Emacs session or add the following to
;; your .emacs to always load this mode (not recommended):
;;
;;   (load-library "lfort-completion-mode")
;;
;; Once you have done this, you can set various parameters with
;;
;;   M-x customize-group RET lfort-completion-mode RET
;;
;; Finally, to try LFort-based code completion in a particular buffer,
;; use M-x lfort-completion-mode. When "LFort" shows up in the mode
;; line, LFort's code-completion is enabled.
;;
;; LFort's code completion is based on parsing the complete source
;; file up to the point where the cursor is located. Therefore, LFort
;; needs all of the various compilation flags (include paths, dialect
;; options, etc.) to provide code-completion results. Currently, these
;; need to be placed into the lfort-flags variable in a format
;; acceptable to lfort. This is a hack: patches are welcome to
;; improve the interface between this Emacs mode and LFort! 
;;

;;; Code:
;;; The lfort executable
(defcustom lfort "lfort"
  "The location of the LFort compiler executable"
  :type 'file
  :group 'lfort-completion-mode)

;;; Extra compilation flags to pass to lfort.
(defcustom lfort-flags nil
  "Extra flags to pass to the LFort executable.
This variable will typically contain include paths, e.g., -I~/MyProject."
  :type '(repeat (string :tag "Argument" ""))
  :group 'lfort-completion-mode)

;;; The prefix header to use with LFort code completion. 
(setq lfort-completion-prefix-header "")

;;; The substring we will use to filter completion results
(setq lfort-completion-substring "")

;;; The current completion buffer
(setq lfort-completion-buffer nil)

(setq lfort-result-string "")

;;; Compute the current line in the buffer	
(defun current-line ()
  "Return the vertical position of point..."
  (+ (count-lines (point-min) (point))
     (if (= (current-column) 0) 1 0)
     -1))

;;; Set the LFort prefix header
(defun lfort-prefix-header ()
  (interactive)
  (setq lfort-completion-prefix-header
        (read-string "LFort prefix header> " "" lfort-completion-prefix-header
                     "")))

;; Process "filter" that keeps track of the code-completion results
;; produced. We store all of the results in a string, then the
;; sentinel processes the entire string at once.
(defun lfort-completion-stash-filter (proc string)
  (setq lfort-result-string (concat lfort-result-string string)))

;; Filter the given list based on a predicate.
(defun filter (condp lst)
    (delq nil
          (mapcar (lambda (x) (and (funcall condp x) x)) lst)))

;; Determine whether FIXME: explain better
(defun is-completion-line (line)
  (or (string-match "OVERLOAD:" line)
      (string-match (concat "COMPLETION: " lfort-completion-substring) line)))


;; re-process the completions when further input narrows the field
(defun lfort-completion-display (buffer)
  (fill-buffer buffer))

(defun fill-buffer (buffer)
  (let* ((all-lines (split-string lfort-result-string "\n"))
         (completion-lines (filter 'is-completion-line all-lines)))
    (if (consp completion-lines)
        (progn
         ;; Erase the process buffer.
         (let ((cur (current-buffer)))
           (set-buffer buffer)
           (goto-char (point-min))
           (erase-buffer)
           (set-buffer cur))
         
         ;; Display the process buffer.
         (display-buffer buffer)
         
         ;; Insert the code-completion string into the process buffer.
         (with-current-buffer buffer
           (insert (mapconcat 'identity completion-lines "\n")))
         ))))

;; Process "sentinel" that, on successful code completion, replaces the 
;; contents of the code-completion buffer with the new code-completion results
;; and ensures that the buffer is visible.
(defun lfort-completion-sentinel (proc event)
  (fill-buffer (process-buffer proc)))

(defun lfort-complete ()
  (let* ((cc-point (concat (buffer-file-name)
                           ":"
                           (number-to-string (+ 1 (current-line)))
                           ":"
                           (number-to-string (+ 1 (current-column)))))
         (cc-pch (if (equal lfort-completion-prefix-header "") nil
                   (list "-include-pch"
                         (concat lfort-completion-prefix-header ".pch"))))
         (cc-flags (if (listp lfort-flags) lfort-flags nil))
         (cc-command (append `(,lfort "-cc1" "-fsyntax-only")
                             cc-flags
                             cc-pch
                             `("-code-completion-at" ,cc-point)
                             (list (buffer-file-name))))
         (cc-buffer-name (concat "*LFort Completion for " (buffer-name) "*")))
    ;; Start the code-completion process.
    (if (buffer-file-name)
        (progn
          ;; If there is already a code-completion process, kill it first.
          (let ((cc-proc (get-process "LFort Code-Completion")))
            (if cc-proc
                (delete-process cc-proc)))

          (setq lfort-completion-substring "")
          (setq lfort-result-string "")
          (setq lfort-completion-buffer cc-buffer-name)
            
          (let ((cc-proc (apply 'start-process
                                (append (list "LFort Code-Completion" cc-buffer-name)
                                        cc-command))))
            (set-process-filter cc-proc 'lfort-completion-stash-filter)
            (set-process-sentinel cc-proc 'lfort-completion-sentinel)
            )))))

;; Code-completion when one of the trigger characters is typed into
;; the buffer, e.g., '(', ',' or '.'.
(defun lfort-complete-self-insert (arg)
  (interactive "p")
  (self-insert-command arg)
  (save-buffer)
  (lfort-complete))

;; When the user has typed a character that requires the filter to be
;; updated, do so (and update the display of results).
(defun lfort-update-filter ()
  (setq lfort-completion-substring (thing-at-point 'symbol))
  (if (get-process "LFort Code-Completion")
      ()
    (lfort-completion-display lfort-completion-buffer)
    ))
  
;; Invoked when the user types an alphanumeric character or "_" to
;; update the filter for the currently-active code completion.
(defun lfort-filter-self-insert (arg)
  (interactive "p")
  (self-insert-command arg)
  (lfort-update-filter)
  )

;; Invoked when the user types the backspace key to update the filter
;; for the currently-active code completion.
(defun lfort-backspace ()
  (interactive)
  (delete-backward-char 1)
  (lfort-update-filter))

;; Invoked when the user types the delete key to update the filter
;; for the currently-active code completion.
(defun lfort-delete ()
  (interactive)
  (delete-backward-char 1)
  (lfort-update-filter))

;; Set up the keymap for the LFort minor mode.
(defvar lfort-completion-mode-map nil
  "Keymap for LFort Completion Mode.")

(if (null lfort-completion-mode-map)
    (fset 'lfort-completion-mode-map 
          (setq lfort-completion-mode-map (make-sparse-keymap))))

(if (not (assq 'lfort-completion-mode minor-mode-map-alist))
    (setq minor-mode-map-alist
          (cons (cons 'lfort-completion-mode lfort-completion-mode-map)
                minor-mode-map-alist)))

;; Punctuation characters trigger code completion.
(dolist (char '("(" "," "." ">" ":" "=" ")" " "))
  (define-key lfort-completion-mode-map char 'lfort-complete-self-insert))

;; Alphanumeric characters (and "_") filter the results of the
;; currently-active code completion.
(dolist (char '("A" "B" "C" "D" "E" "F" "G" "H" "I" "J" "K" "L" "M" "N" "O"
                "P" "Q" "R" "S" "T" "U" "V" "W" "X" "Y" "Z"
                "a" "b" "c" "d" "e" "f" "g" "h" "i" "j" "k" "l" "m" "n" "o"
                "p" "q" "r" "s" "t" "u" "v" "w" "x" "y" "z"
                "_" "0" "1" "2" "3" "4" "5" "6" "7" "8" "9"))
  (define-key lfort-completion-mode-map char 'lfort-filter-self-insert))

;; Delete and backspace filter the results of the currently-active
;; code completion.
(define-key lfort-completion-mode-map [(backspace)] 'lfort-backspace)
(define-key lfort-completion-mode-map [(delete)] 'lfort-delete)

;; Set up the LFort minor mode.
(define-minor-mode lfort-completion-mode 
  "LFort code-completion mode"
  nil
  " LFort"
  lfort-completion-mode-map)

