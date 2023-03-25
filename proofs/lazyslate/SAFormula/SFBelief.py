from .Parser import parse_fstring
from .Belief import Belief

# fstring        -- String                -- String representation of formula (in s-expression style) (e.g. (Believes!4 alice now phi))
# fstring_no_ann -- String                -- String form with no strength factor 
# justification  -- Justification || None -- Links the formula to its justification
# agent          -- String
# time           -- String
# formula        -- Formula               -- Sub-formula (object of modal operator)
class SFBelief(Belief):

  def __init__(self, fstring, justification, agent, time, formula, strength):
    super().__init__(fstring, justification, agent, time, formula)
    self.strength = strength
    self.fstring_no_ann = fstring[:10] + fstring[11:]



  @classmethod
  def from_string(cls, fstring, justification=None):
    # Split off the agent and time, leaving the sub-formula intact
    args = fstring.split(maxsplit=3)

    agent = args[1]
    time  = args[2]
    formula = parse_fstring(args[3][:-1]) # Pass sub-formula to parser

    try:
      strength = int(fstring[10])

    except ValueError:
      print("Improperly formatted SF belief (invalid strength factor)")
      exit(1)

    return cls(fstring, justification, agent, time, formula, strength)



  @classmethod
  def from_args(cls, agent, time, formula, strength, justification=None):
    fstring = "(Believes!" + str(strength) + " " + agent + " " + time + " " + str(formula) + ")"
    return cls(fstring, justification, agent, time, formula, strength)



  def is_annotated(self):
    return True



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

