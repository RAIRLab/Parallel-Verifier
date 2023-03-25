from .Parser  import parse_fstring, parse_sexpr
from .Formula import Formula

modalities = ["Perceives", "Believes", "Knows", "Says", "Intends", "Desires", "Ought"]

# fstring       -- String                -- String representation of formula (in s-expression style) (e.g. (Happy john))
# justification -- Justification || None -- Links the formula to its justification
# name          -- String                -- Name of predicate (e.g. "Happy")
# args          -- List(Formula)         -- Sub-formulae (arguments to predicate) (e.g. ["john"])
class Predicate(Formula):

  def __init__(self, fstring, justification, name, args):

    # Catch modal operators accidentally entered as predicates
    if(name in modalities):
      print("I think you're missing a '!' after '" + name + "'.")

    super().__init__(fstring, justification)
    self.name = name
    self.args = args



  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring) # Parse s-expression
    name = args[0]
    args = args[1:]             # Remove predicate name (e.g. "Happy")

    # Recursively parse sub-formulae
    init_args = []
    for a in args:
      init_args.append(parse_fstring(a))

    return cls(fstring, justification, name, init_args)



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

