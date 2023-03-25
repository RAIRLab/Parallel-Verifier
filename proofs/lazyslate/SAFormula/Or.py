from .Proposition import Proposition

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# args          -- List(Formula)         -- Sub-formulae (arguments to proposition)
class Or(Proposition):

  def __init__(self, fstring, justification, args):
    super().__init__(fstring, justification, args)



  @classmethod
  def from_string(cls, fstring, justification=None):
    return super().from_string(fstring, justification)



  @classmethod
  def from_args(cls, disjuncts, justification=None):
    fstring = "(or "

    for d in disjuncts:
      fstring += str(d) + " "
    fstring = fstring[:-1] + ")" # Removes trailing space

    return cls(fstring, justification, disjuncts)



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()
