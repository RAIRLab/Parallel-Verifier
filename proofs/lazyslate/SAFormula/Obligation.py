from .Modal  import Modal
from .Parser import parse_sexpr, parse_fstring

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# agent         -- String
# time          -- String
# precondition  -- Formula               -- Formula that, if true, binds the agent to the obligation
# formula       -- Formula               -- Sub-formula (object of modal operator)
class Obligation(Modal):

  def __init__(self, fstring, justification, agent, time, precondition, formula):
    super().__init__(fstring, justification, agent, time, formula)
    self.precondition = precondition



  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring)

    agent        = args[1]
    time         = args[2]
    precondition = parse_fstring(args[3])
    formula      = parse_fstring(args[4])

    return cls(fstring, justification, agent, time, precondition, formula)



  @classmethod
  def from_args(cls, agent, time, formula, precondition, justification=None):
    fstring = "(Ought! " + agent + " " + time + " " + str(precondition) + " " + str(formula) + ")"
    return cls(fstring, justification, agent, time, precondition, formula)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

