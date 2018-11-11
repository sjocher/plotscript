(begin
    (define make-point (lambda (x y) (list x y)))
    (define make-point (set-property "object-name" "point" make-point))
    (define make-line (lambda (p1 p2) (list p1 p2)))
    (define make-line (set-property "object-name" "line" make-line))
    (define make-text (lambda (string) (string)))
    (define make-text (set-property "object-name" "text" make-text))
    (define make-text (set-property "rotation" 0 make-text))
)
