from abc import abstractmethod

from .Parser  import parse_fstring, parse_sexpr
from .Formula import Formula

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# args          -- List(Formula)         -- Sub-formulae (arguments to proposition)
class Proposition(Formula):

  def __init__(self, fstring, justification, args):
    super().__init__(fstring, justification)
    self.args = args



  # Parses and recursively instantiates objects for all sub-arguments
  # Useful for all sub-classes (e.g. and, implies) EXCEPT ATOM
  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring) # Parse s-expression
    args = args[1:]             # Remove proposition type (e.g. "and", "implies")

    # Recursively parse sub-formulae
    init_args = []
    for a in args:
      init_args.append(parse_fstring(a))

    return cls(fstring, justification, init_args)



  # Each sub-class will need to define this on its own
  @classmethod
  @abstractmethod # TODO No idea if this is allowed...
  def from_args(cls, args, justification=None):
    pass



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

