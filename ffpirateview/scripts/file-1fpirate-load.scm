;;attempted to install <Load> procedure "file-1fpirate-load" which does not take the standard <Load> plug-in's arguments: (IMAGE).

;;Plug-in "script-fu"
;;(/usr/lib/gimp/2.0/plug-ins/script-fu/script-fu) is installing procedure "file-1fpirate-save" with a full menu path "<Save>/1fpirate" as menu label, this deprecated and will be an error in GIMP 3.0
;;gimp: LibGimpBase-CRITICAL: gimp_value_array_index: assertion 'index < value_array->n_values' failed
;;gimp: Gimp-Core-CRITICAL: gimp_value_get_image: assertion 'GIMP_VALUE_HOLDS_IMAGE_ID (value)' failed
;;gimp: LibGimpBase-CRITICAL: gimp_value_array_index: assertion 'index < value_array->n_values' failed
;;gimp: Gimp-Core-CRITICAL: gimp_value_get_image: assertion 'GIMP_VALUE_HOLDS_IMAGE_ID (value)' failed


(define (script-fu-load-1fpirate filename raw-filename)
  (let* (
         (img-width 256)   ;; Placeholder: You must parse actual width
         (img-height 256)  ;; Placeholder: You must parse actual height
         (image (car (gimp-image-new img-width img-height RGB)))
         (layer (car (gimp-layer-new image img-width img-height RGB "1f Layer" 100 NORMAL-MODE)))
        )

    (gimp-image-insert-layer image layer 0 -1)
    
    ;; load your 1-bit data from file and draw into the layer using gimp-drawable-set-pixel
    
    ;; Just a placeholder example:
    (gimp-drawable-fill layer WHITE-FILL)

    (gimp-image-set-filename image filename)
    image
  )
)

(script-fu-register
  "file-1fpirate-load"
  "<Load>/1fpirate"
  "Loads 1fpirate format images"
  "Your Name"
  "Your License"
  "2025"
  ""
  SF-FILENAME "Filename" ""
  SF-STRING   "Raw Filename" ""
)

(script-fu-register-load-handler "file-1fpirate-load" "1f" "1ff")
