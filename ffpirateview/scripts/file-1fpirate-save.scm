(define (script-fu-save-1fpirate image drawable filename raw-filename)
  ;; extract pixel data from drawable
  ;; and write it in 1-bit packed format to `filename`
  (let* (
         (width (car (gimp-image-width image)))
         (height (car (gimp-image-height image)))
        )

    ;; just an example: create a blank file with the MAGIC
    (let ((port (open-output-file filename)))
      (display "1fpirate" port) ;; the MAGIC, no null terminator
      ;; write width/height (fake placeholder)
      (put-u32-be port width)
      (put-u32-be port height)
      ;; write dummy pixel bytes (just white for now)
      (let loop ((i 0))
        (if (< i (* height (ceiling (/ width 8.0))))
            (begin (write-byte 0xFF port) (loop (+ i 1)))
        )
      )
      (close-output-port port)
    )
  )
)

(script-fu-register
  "file-1fpirate-save"
  "<Save>/1fpirate"
  "Saves 1fpirate format images"
  "Your Name"
  "Your License"
  "2025"
  ""
  SF-IMAGE    "Image" 0
  SF-DRAWABLE "Drawable" 0
  SF-FILENAME "Filename" ""
  SF-STRING   "Raw Filename" ""
)

(script-fu-register-save-handler "file-1fpirate-save" "1f" "1ff")
