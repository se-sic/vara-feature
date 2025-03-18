grammar UVLcpp;
import UVLBase;

OPEN_PAREN : '(' {self.opened += 1;};
CLOSE_PAREN : ')' {self.opened -= 1;};
OPEN_BRACK : '[' {self.opened += 1;};
CLOSE_BRACK : ']' {self.opened -= 1;};
OPEN_BRACE : '{' {self.opened += 1;};
CLOSE_BRACE : '}' {self.opened -= 1;};
OPEN_COMMENT: '/*' {self.opened += 1;};
CLOSE_COMMENT: '*/' {self.opened -= 1;};

