from .Modal   import Modal
from .Parser  import parse_fstring

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# agent         -- String
# time          -- String
# formula       -- Formula               -- Sub-formula (object of modal operator)
class Says(Modal):

  def __init__(self, fstring, justification, agent, listener, time, formula):
    super().__init__(fstring, justification, agent, time, formula)
    self.listener = listener
    self.fstring_no_listener = "(Says! " + agent + " " + time + " " + str(formula) + ")"



  @classmethod
  def from_string(cls, fstring, justification=None):
    # Split off the agent, listener, and time, leaving the sub-formula intact
    args = fstring.split(maxsplit=4)

    agent = args[1]
    listener = args[2]
    time  = args[3]
    formula = parse_fstring(args[4][:-1]) # Pass sub-formula to parser

    return cls(fstring, justification, agent, listener, time, formula)



  @classmethod
  def from_args(cls, agent, time, formula, justification=None):
    fstring = "(Says! " + agent + " " + listener + " " + time + " " + str(formula) + ")"
    return cls(fstring, justification, agent, time, formula)



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

