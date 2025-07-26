;; deepseek version
;; also des not work

(define (script-fu-load-1fpirate filename raw-filename)
  (let* (
         (fp (open-input-file filename))
         (magic (make-string 8))
         (width 0)
         (height 0)
         (image)
         (layer)
        )

    ;; Read magic
    (read-string! magic fp)
    (if (not (string=? magic "1bfarbfe"))
        (begin
          (gimp-message "Not a 1fpirate file.")
          (close-input-port fp)
          (error "Bad magic header")))

    ;; Read big-endian width and height
    (define (read-u32)
      (let ((b1 (char->integer (read-char fp)))
            (b2 (char->integer (read-char fp)))
            (b3 (char->integer (read-char fp)))
            (b4 (char->integer (read-char fp))))
        (+ (ash b1 24) (ash b2 16) (ash b3 8) b4)))

    (set! width (read-u32))
    (set! height (read-u32))

    (set! image (car (gimp-image-new width height GRAY)))
    (set! layer (car (gimp-layer-new image width height GRAY-IMAGE "1f Layer" 100 NORMAL-MODE)))
    (gimp-image-insert-layer image layer -1 0)

    (let* ((bytes-per-row (ceiling (/ width 8.0)))
           (pr (gimp-drawable-get-pixel-rgn layer 1 1)))
      (do ((y 0 (+ y 1)))
          ((= y height))
        (let ((row-bytes (read-bytes bytes-per-row fp)))
          (do ((x 0 (+ x 1)))
              ((= x width))
            (let* ((byte-index (quotient x 8))
                   (bit-index (- 7 (modulo x 8)))
                   (bit (if (> (bitwise-and (vector-ref row-bytes byte-index) (ash 1 bit-index)) 0) 0 255)))
              (gimp-drawable-set-pixel layer x y 1 (cons bit '())))))))

    (gimp-drawable-update layer 0 0 width height)
    (close-input-port fp)
    image))

(script-fu-register
 "script-fu-load-1fpirate"
 "Open 1fpirate image (.1f)"
 "Loads a 1-bit custom .1f image format"
 "You"
 "Public Domain"
 "2025"
 ""
 SF-FILENAME "1f Image" ""
 SF-FILENAME "Raw Filename" ""
)

(script-fu-menu-register "script-fu-load-1fpirate" "<Load>")
