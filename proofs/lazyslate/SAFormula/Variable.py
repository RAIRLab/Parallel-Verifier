from .Proposition import Proposition

# name -- String -- Variable name
class Variable():

  def __init__(self, name):
    if(not name.startswith("?")):
      print("ERROR: Improperly formatted variable: " + name)
      exit(1)
    self.name = name[1:]



  def __str__(self):
    return "?" + self.name

