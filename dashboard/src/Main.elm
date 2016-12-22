module Main exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Http
import Json.Decode exposing (Decoder, float, int, string)
import Json.Decode.Pipeline exposing (decode, hardcoded, optional, required, requiredAt)
import Task
import Model exposing (..)
import User


-- initial model and commands generator


init : Flags -> ( Model, Cmd Msg )
init flags =
    let
        initialModel =
            { user = Nothing
            , userId = 0
            , isLoading = False
            , error = Nothing
            , swapCount = flags.swapCount
            }
    in
        Debug.log "init"
            ( initialModel, Cmd.none )



-- list of possible actions


type Msg
    = LoadUser
    | UserResult (Result Http.Error User)



-- http actions


loadUser : Int -> Cmd Msg
loadUser id =
    let
        url =
            "https://reqres.in/api/users/" ++ (toString id) ++ "?delay=1"

        request =
            Http.get url userDecoder
    in
        Http.send UserResult request


loadFirstUser : Cmd Msg
loadFirstUser =
    Task.perform identity (Task.succeed LoadUser)



-- http json response decoders for a response like
-- {"data":{"id":1,"first_name":"george","last_name":"bluth","avatar":"https://s3.amazonaws.com/uifaces/faces/twitter/calebogden/128.jpg"}}


userDecoder : Decoder User
userDecoder =
    decode User
        |> requiredAt [ "data", "id" ] int
        |> requiredAt [ "data", "first_name" ] string
        |> requiredAt [ "data", "last_name" ] string
        |> requiredAt [ "data", "avatar" ] string



-- renders the view based on model


view : Model -> Html Msg
view model =
    div []
        [ img [ src "assets/logo.png" ] []
        , h1 [] [ text "Elm HTTP request playground" ]
        , button [ onClick LoadUser ] [ text "Load next user" ]
        , viewUser model
        , em [] [ text ("parts of this application have been reloaded " ++ (toString model.swapCount) ++ " times") ]
        , p [] [ text (toString model) ]
        ]


viewUser : Model -> Html Msg
viewUser model =
    case model.error of
        Just error ->
            viewError error

        Nothing ->
            if model.isLoading then
                viewLoading model
            else
                User.viewUserInfo model.user


viewLoading : Model -> Html Msg
viewLoading model =
    div []
        [ h2 [] [ text "Loading user" ]
        , div [] [ text ("Loading user #" ++ (toString model.userId) ++ " info") ]
        ]


viewError : Http.Error -> Html Msg
viewError error =
    div [ class "error" ]
        [ h2 [] [ text "Error occured" ]
        , div [] [ text (getErrorMessage error) ]
        ]


getErrorMessage : Http.Error -> String
getErrorMessage error =
    case error of
        Http.Timeout ->
            "request timed out"

        Http.NetworkError ->
            "network error occured"

        Http.BadUrl url ->
            "invalid url '" ++ url ++ "' provided"

        Http.BadStatus response ->
            "request failed with " ++ (toString response.status.code) ++ " (" ++ response.status.message ++ ")"

        Http.BadPayload problem response ->
            "got invalid response (" ++ problem ++ ")"



-- updates the model based on given message


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        LoadUser ->
            let
                newUserId =
                    model.userId + 1
            in
                ( { model | userId = newUserId, isLoading = True, error = Nothing }, loadUser newUserId )

        UserResult (Ok loadedUser) ->
            ( { model | user = Just loadedUser, isLoading = False, error = Nothing }, Cmd.none )

        UserResult (Err error) ->
            -- todo: handle error
            ( { model | error = Just error, isLoading = False }, Cmd.none )



-- subscriptions


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.none



-- main program


main : Program Flags Model Msg
main =
    Html.programWithFlags
        { init = init
        , view = view
        , update = update
        , subscriptions = subscriptions
        }
