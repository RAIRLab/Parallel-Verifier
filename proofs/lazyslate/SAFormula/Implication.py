from .Proposition import Proposition

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# args          -- List(Formula)         -- Sub-formulae (arguments to proposition)
class Implication(Proposition):

  def __init__(self, fstring, justification, args):
    assert(len(args) == 2)
    super().__init__(fstring, justification, args)



  @classmethod
  def from_string(cls, fstring, justification=None):
    return super().from_string(fstring, justification)



  @classmethod
  def from_args(cls, left, right, justification=None):
    fstring = "(implies " + str(left) + " " + str(right) + ")"
    args = [left, right]
    return cls(fstring, justification, args)



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()
