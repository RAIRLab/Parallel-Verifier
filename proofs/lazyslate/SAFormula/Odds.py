from .Parser    import parse_fstring, parse_sexpr
from .Predicate import Predicate

# fstring       -- String                -- String representation of formula (in s-expression style) (e.g. (Odds! phi (case1) (case2 case3)))
# justification -- Justification || None -- Links the formula to its justification
# name          -- String                -- Name of predicate (e.g. "Happy")
# args          -- List(Formula)         -- Sub-formulae (arguments to predicate) (e.g. ["john"])
class Odds(Predicate):

  def __init__(self, fstring, justification, formula, positive, negative):

    name = "Odds!"
    args = [positive, negative]

    super().__init__(fstring, justification, name, args)

    self.formula  = formula
    self.positive = positive
    self.negative = negative



  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring) # Parse s-expression

    formula  = parse_fstring(args[1])
    positive = parse_sexpr(args[2])[1:] # Strip off "POS"
    negative = parse_sexpr(args[3])[1:] # Strip off "NEG"

    return cls(fstring, justification, formula, positive, negative)



  @classmethod
  def from_args(cls, formula, positive, negative, justification=None):
    fstring = "(Odds! " + str(formula) + " ("

    for p in positive:
      fstring += p + " "
    fstring = fstring[:-1] + ") (" # Removes trailing space

    for n in negative:
      fstring += n + " "
    fstring = fstring[:-1] + "))" # Removes trailing space

    return cls(fstring, justification, formula, positive, negative)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__() 



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

