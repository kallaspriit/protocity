module Component.User exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Model.User exposing (..)


viewUserInfo : Maybe User -> Html msg
viewUserInfo user =
    case user of
        Just user ->
            div [ class "user" ]
                [ h2 [] [ text "User info" ]
                , div [ class "name" ] [ text (user.firstName ++ " " ++ user.lastName) ]
                , div [ class "avatar" ] [ img [ src user.avatar ] [] ]
                ]

        Nothing ->
            div [] [ text ("User not loaded") ]
