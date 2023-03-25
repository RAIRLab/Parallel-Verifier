from .Parser     import parse_fstring, parse_sexpr
from .Quantifier import Quantifier

# fstring       -- String                -- String representation of formula (in s-expression style) (e.g. (Happy john))
# justification -- Justification || None -- Links the formula to its justification
# quant_vars    -- List(Atom)            -- List of variables (e.g. ["x"])
# formula       -- Formula               -- Sub-formula (e.g. "(Happy x)")
class Universal(Quantifier):

  def __init__(self, fstring, justification, quant_vars, formula):
    super().__init__(fstring, justification, quant_vars, formula)



  @classmethod
  def from_string(cls, fstring, justification=None):
    return super().from_string(fstring, justification)



  @classmethod
  def from_args(cls, quant_vars, formula, justification=None):
    fstring = "(forall ["

    for v in quant_vars:
      fstring += "?" + str(v) + " "
    fstring = fstring[:-1] + "]" # Removes trailing space

    return cls(fstring, justification, quant_vars, formula)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

