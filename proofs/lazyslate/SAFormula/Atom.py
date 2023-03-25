from .Proposition import Proposition

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# args          -- List(Formula)         -- Sub-formulae (for an Atom, args = [fstring])
class Atom(Proposition):

  def __init__(self, fstring, justification, args):
    super().__init__(fstring, justification, args)



  # A propositional atom is simply represented by its fstring
  @classmethod
  def from_string(cls, fstring, justification=None):
    args = [fstring]
    return cls(fstring, justification, args)



  # Same as from_string, since the only argument of an Atom is its name
  # No reason to use this function, but it needs to be implemented to
  # satisfy abstract class (and I believe this is a unique exception...)
  @classmethod
  def from_args(cls, arg, justification=None):
    fstring = arg
    args = [arg]
    return cls(fstring, justification, args)



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

