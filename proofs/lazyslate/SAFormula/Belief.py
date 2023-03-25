from .Modal import Modal

# fstring       -- String                -- String representation of formula (in s-expression style) 
# justification -- Justification || None -- Links the formula to its justification
# agent         -- String
# time          -- String
# formula       -- Formula               -- Sub-formula (object of modal operator)
class Belief(Modal):

  def __init__(self, fstring, justification, agent, time, formula):
    super().__init__(fstring, justification, agent, time, formula)



  @classmethod
  def from_string(cls, fstring, justification=None):
    return super().from_string(fstring, justification)



  @classmethod
  def from_args(cls, agent, time, formula, justification=None):
    fstring = "(Believes! " + agent + " " + time + " " + str(formula) + ")"
    return cls(fstring, justification, agent, time, formula)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

