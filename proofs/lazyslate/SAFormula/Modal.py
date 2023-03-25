from .Formula import Formula

from abc import abstractmethod



# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
# agent         -- String
# time          -- String
# formula       -- Formula               -- Sub-formula (object of modal operator)
class Modal(Formula):

  def __init__(self, fstring, justification, agent, time, formula):
    super().__init__(fstring, justification)
    self.agent   = agent
    self.time    = time
    self.formula = formula



  @classmethod
  def from_string(cls, fstring, justification=None):
    from .Parser  import parse_fstring # Needs to be in function to avoid circular import

    # Split off the agent and time, leaving the sub-formula intact
    args = fstring.split(maxsplit=3)

    agent = args[1]
    time  = args[2]
    formula = parse_fstring(args[3][:-1]) # Pass sub-formula to parser

    return cls(fstring, justification, agent, time, formula)



  # Each sub-class will need to define this on its own
  @classmethod
  @abstractmethod # TODO No idea if this is allowed...
  def from_args(cls, args, justification=None):
    pass



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return isinstance(other, Modal) and type(self) is type(other) and self.agent == other.agent and self.time == other.time and self.formula == other.formula



  def __hash__(self):
    return super().__hash__()
