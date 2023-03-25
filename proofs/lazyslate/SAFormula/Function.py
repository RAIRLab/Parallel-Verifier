from .Parser  import parse_fstring, parse_sexpr
from .Formula import Formula

# fstring       -- String                 -- String representation of formula (in s-expression style) (e.g. (safety a t lga))
# justification -- Justification || None  -- Links the formula to its justification
# name          -- String                 -- Name of function (e.g. "safety")
# args          -- List(Formula)          -- Sub-formulae (arguments to function) (e.g. ["a", "t", "lga"])
class Function(Formula):

  def __init__(self, fstring, justification, name, args):
    super().__init__(fstring, justification)
    self.name = name
    self.args = args



  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring) # Parse s-expression
    name = args[0]
    args = args[1:]             # Remove function name (e.g. ">")

    # Recursively parse sub-formulae
    init_args = []
    for a in args:
      init_args.append(parse_fstring(a))

    if(cls == Function):
      return cls(fstring, justification, name, init_args)
    # Sub-classes to function should not take a name parameter (they know their own name)
    else:
      return cls(fstring, justification, init_args)



  @classmethod
  def from_args(cls, name, args, justification=None):
    fstring = "(" + name + " "

    for a in args:
      fstring += str(a) + " "
    fstring = fstring[:-1] + ")" # Removes trailing space

    return cls(fstring, justification, name, args)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

