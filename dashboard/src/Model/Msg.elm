module Model.Msg exposing (..)

import Http
import Model.User exposing (..)


type Msg
    = LoadUser
    | UserResult (Result Http.Error User)
